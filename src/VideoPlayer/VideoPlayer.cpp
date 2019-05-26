#include "VideoPlayer.h"

PacketQueue::PacketQueue()
{
	first_pkt = NULL;
	last_pkt = NULL;
	nb_packets = 0;
	size = 0;
	capacity = 1024;
	mutex = SDL_CreateMutex();
	cond = SDL_CreateCond();
	full = SDL_CreateCond();
}

int PacketQueue::pushQueue(AVPacket *pkt)
{
	AVPacketList *pkt1;
	if (av_dup_packet(pkt) < 0)
	{
		return -1;
	}
	pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;


	SDL_LockMutex(mutex);
	if (nb_packets == capacity)
	{
		//printf("PUTwaitting...");
		SDL_CondWait(full, mutex);
	}
	if (!last_pkt)
		first_pkt = pkt1;
	else
		last_pkt->next = pkt1;
	last_pkt = pkt1;
	nb_packets++;
	size += pkt1->pkt.size;
	//printf("put%d\n", nb_packets);
	SDL_CondSignal(cond);

	SDL_UnlockMutex(mutex);
	return 0;
}

int PacketQueue::getQueue(AVPacket *pkt)
{
	AVPacketList *pkt1;
	int ret;
	SDL_LockMutex(mutex);
	for (;;)
	{
		if (nb_packets == capacity)
		{
			//printf("GETrelease...\n");
			SDL_CondSignal(full);
		}
		pkt1 = first_pkt;
		if (pkt1)
		{
			first_pkt = pkt1->next;
			if (!first_pkt)
				last_pkt = NULL;
			nb_packets--;
			size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			//printf("get%d\n", nb_packets);
			av_free(pkt1);
			ret = 1;
			break;
		}
		else
		{
			//printf("%d",nb_packets);
			//printf("GETwaitting...\n");
			SDL_CondWait(cond, mutex);
		}
	}
	SDL_UnlockMutex(mutex);
	return ret;
}

VideoPlayer::VideoPlayer()
{
	qAudio = new PacketQueue();
	qVideo = new PacketQueue();
	finishRead = false;
	audioBuffer = (uint8_t *)av_malloc(2 * 44100);
	packetAudio = (AVPacket *)av_malloc(sizeof(AVPacket));
	frame = av_frame_alloc();
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
	}
}

VideoPlayer::~VideoPlayer()
{
	av_free_packet(packetAudio);
	av_free(audioBuffer);
	av_frame_free(&frame);
	//�ͷ��ڴ�
	sws_freeContext(img_convert_ctx);
	SDL_CloseAudio();//�ر���Ƶ�豸 
	SDL_Quit();
	swr_free(&swrCtx);
	avcodec_close(pAudioCodeCtx);
	avcodec_close(pVideoCodeCtx);
	avformat_close_input(&pFormatCtx);
}

int VideoPlayer::openFile(char *filepath)
{
	//�������ļ�
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
	//��ȡ�ļ���Ϣ
	if (avformat_find_stream_info(pFormatCtx, NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}

	//�ҵ���Ӧ��type���ڵ�pFormatCtx->streams������λ��
	indexAudio = indexVideo = -1;
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			indexAudio = i;
		}
		else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			indexVideo = i;
		}
		if (indexAudio != -1 && indexVideo != -1)
		{
			break;
		}
	}
		
	if (indexAudio == -1 || indexVideo == -1)
	{
		printf("Didn't find a video stream.\n");
	}
	//��ȡ������
	pAudioCodeCtx = pFormatCtx->streams[indexAudio]->codec;
	pVideoCodeCtx = pFormatCtx->streams[indexVideo]->codec;

	AVCodec *pAudioCodec = avcodec_find_decoder(pAudioCodeCtx->codec_id);
	AVCodec *pVideoCodec = avcodec_find_decoder(pVideoCodeCtx->codec_id);
	if (pAudioCodec == NULL || pVideoCodec == NULL){
		printf("Codec not found.\n");
		return -1;
	}

	//�򿪽�����
	if (avcodec_open2(pAudioCodeCtx, pAudioCodec, NULL)<0 ||
		avcodec_open2(pVideoCodeCtx, pVideoCodec, NULL)<0)
	{
		printf("Could not open codec.\n");
		return -1;
	}
}

static Uint8 *audio_chunk;
//������Ƶ���ݳ���
static Uint32 audio_len;
static Uint8 *audio_pos;
void  VideoPlayer::fill_audio(void *udata, Uint8 *stream, int len){
	//SDL 2.0
	SDL_memset(stream, 0, len);
	if (audio_len == 0)		//�����ݲŲ���
		return;
	len = (len>audio_len ? audio_len : len);

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
	
}

void VideoPlayer::audioSetting()
{
	//�ز�������ѡ��-----------------------------------------------------------start
	//����Ĳ�����ʽ
	enum AVSampleFormat in_sample_fmt = pAudioCodeCtx->sample_fmt;
	//����Ĳ�����ʽ 16bit PCM
	out_sample_fmt = AV_SAMPLE_FMT_S16;
	//����Ĳ�����
	int in_sample_rate = pAudioCodeCtx->sample_rate;
	//����Ĳ�����
	int out_sample_rate = 44100;
	//�������������
	uint64_t in_ch_layout = pAudioCodeCtx->channel_layout;
	if (in_ch_layout <= 0)
	{
		in_ch_layout = av_get_default_channel_layout(pAudioCodeCtx->channels);
	}
	//�������������
	uint64_t out_ch_layout = AV_CH_LAYOUT_MONO;

	//frame->16bit 44100 PCM ͳһ��Ƶ������ʽ�������
	swrCtx = swr_alloc();
	swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt,
		in_sample_rate, 0, NULL);
	swr_init(swrCtx);
	//�ز�������ѡ��-----------------------------------------------------------end

	//��ȡ�������������
	out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
	//SDL_AudioSpec
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channel_nb;
	wanted_spec.silence = 0;
	wanted_spec.samples = pAudioCodeCtx->frame_size;
	wanted_spec.callback = fill_audio;//�ص�����
	wanted_spec.userdata = this;
	if (SDL_OpenAudio(&wanted_spec, NULL)<0){
		printf("can't open audio.\n");
	}
	//�ط���Ƶ���� 
	SDL_PauseAudio(0);
}

int VideoPlayer::decodeAudio()
{
	if (qAudio->getQueue(packetAudio))
	{
		int ret, got_frame, framecount = 0;
			
		ret = avcodec_decode_audio4(pAudioCodeCtx, frame, &got_frame, packetAudio);
		ptsAudio = av_q2d(pFormatCtx->streams[indexAudio]->time_base) * packetAudio->pts * 1000;
		if (ret < 0) {
			printf("%s", "�������");
		}
		int out_buffer_size;
		if (got_frame) {
			//printf("����%d֡", framecount++);
			swr_convert(swrCtx, &audioBuffer, 2 * 44100, (const uint8_t **)frame->data, frame->nb_samples);
			//��ȡsample��size
			out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, frame->nb_samples,
				out_sample_fmt, 1);
			//������Ƶ���ݳ���
			audio_pos = (Uint8 *)audioBuffer;
			audio_len = out_buffer_size;
		}
	}
	return 0;
}

void VideoPlayer::showInfo()
{
	printf("---------------- File Information ---------------\n");
	av_dump_format(pFormatCtx, 0, "", 0);
	printf("-------------------------------------------------\n");
}

int VideoPlayer::setWindow(const void *data)
{
	int screen_w = 800;
	int screen_h = 600;
	/*SDL_Window *screen = SDL_CreateWindow("FFmpeg player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_RESIZABLE);*/
	SDL_Window *screen = SDL_CreateWindowFrom(data);

	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pVideoCodeCtx->width, pVideoCodeCtx->height);
	//ͼ��ɫ�ʿռ�ת��
	img_convert_ctx = sws_getContext(pVideoCodeCtx->width, pVideoCodeCtx->height, pVideoCodeCtx->pix_fmt,
		pVideoCodeCtx->width, pVideoCodeCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
}

int VideoPlayer::read_thread(void *opaque)
{
	VideoPlayer *pVideoPlayer = (VideoPlayer *)opaque;
	//��������
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//һ֡һ֡��ȡѹ������Ƶ����AVPacket
	while (av_read_frame(pVideoPlayer->pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == pVideoPlayer->indexAudio)
		{
			pVideoPlayer->qAudio->pushQueue(packet);
		}
		else if (packet->stream_index == pVideoPlayer->indexVideo)
		{
			pVideoPlayer->qVideo->pushQueue(packet);
		}
	}
	av_free_packet(packet);
	pVideoPlayer->finishRead = true;
	printf("finishRead\n");
	return 0;
}

int VideoPlayer::sfp_refresh_thread(void *opaque)
{
	VideoPlayer *pMusicPlayer = (VideoPlayer *)opaque;
	while (1)
	{
		if (pMusicPlayer->finishRead && pMusicPlayer->qVideo->size == 0)
		{
			/*SDL_Event event;
			event.type = SFM_BREAK_EVENT;
			SDL_PushEvent(&event);*/
			printf("finishsfp_refresh_thread...\n");
			break;
		}
		if (audio_len == 0)
			pMusicPlayer->decodeAudio();
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
	}
	return 0;
}

int VideoPlayer::decodeVideo()
{
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//�ڴ����
	AVFrame	*pFrame = av_frame_alloc();
	AVFrame	*pFrameYUV = av_frame_alloc();
	//�����������ڴ�
	unsigned char *videoBuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pVideoCodeCtx->width, pVideoCodeCtx->height, 1));
	while (1)
	{
		if (finishRead && qVideo->size == 0)
		{
			printf("outqVideo\n");
			break;
		}
		if (qVideo->getQueue(packet))
		{
			double pts;
			int ret, got_picture;
			ret = avcodec_decode_video2(pVideoCodeCtx, pFrame, &got_picture, packet);
			if (packet->dts != AV_NOPTS_VALUE)
			{
				pts = av_frame_get_best_effort_timestamp(pFrame)*av_q2d(pFormatCtx->streams[indexVideo]->time_base) * 1000;
			}
			else
			{
				pts = 0;
			}
			if (ret < 0){
				printf("Decode Error.\n");
			}
			if (got_picture)
			{
				//��ʼ��������
				av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, videoBuffer,
					AV_PIX_FMT_YUV420P, pVideoCodeCtx->width, pVideoCodeCtx->height, 1);
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pVideoCodeCtx->height, pFrameYUV->data, pFrameYUV->linesize);
				
				while (1)
				{
					//printf("in\n");
					SDL_WaitEvent(&event);
					if (event.type == SFM_REFRESH_EVENT)
					{
						if (pts > ptsAudio)
						{
							//SDL_Delay(1000 / frameRate);
							SDL_Delay(pts-ptsAudio);
						}
						//SDL---------------------------
						SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
						SDL_RenderClear(sdlRenderer);
						SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
						SDL_RenderPresent(sdlRenderer);
						//SDL End-----------------------
						break;
					}
					//printf("out\n");
				}
			}
			av_free_packet(packet);
		}
	}
	av_free(videoBuffer);
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	
	return 0;
}

int VideoPlayer::play()
{
	SDL_CreateThread(read_thread, NULL, this);
	SDL_CreateThread(sfp_refresh_thread, NULL, this);
	decodeVideo();
	return 0;
}

