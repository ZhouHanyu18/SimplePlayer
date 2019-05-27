#include "player.h"

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
		SDL_CondWait(full, mutex);
	}
	if (!last_pkt)
		first_pkt = pkt1;
	else
		last_pkt->next = pkt1;
	last_pkt = pkt1;
	nb_packets++;
	size += pkt1->pkt.size;
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
			av_free(pkt1);
			ret = 1;
			break;
		}
		else
		{
			SDL_CondWait(cond, mutex);
		}
	}
	SDL_UnlockMutex(mutex);
	return ret;
}

void PacketQueue::initQueue()
{
	AVPacketList *pkt, *pkt1;

	SDL_LockMutex(mutex);
	SDL_CondSignal(full);
	for (pkt = first_pkt; pkt != NULL; pkt = pkt1) {
		pkt1 = pkt->next;
		av_free_packet(&pkt->pkt);
		av_freep(&pkt);
	}
	last_pkt = NULL;
	first_pkt = NULL;
	nb_packets = 0;
	size = 0;
	SDL_UnlockMutex(mutex);
}


Player::Player()
{
	av_register_all();
	avformat_network_init();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
	}
	pFormatCtx = avformat_alloc_context();
	swrCtx = swr_alloc();
}

Player::~Player()
{
	SDL_WaitThread(pReadThread, NULL);
	SDL_WaitThread(pDecodeThread, NULL);
	swr_free(&swrCtx);
	SDL_CloseAudio();//�ر���Ƶ�豸 
	SDL_Quit();
	avformat_close_input(&pFormatCtx);
}

int Player::openFile(char *filepath)
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
	return 0;
}

void Player::showInfo()
{
	printf("---------------- File Information ---------------\n");
	av_dump_format(pFormatCtx, 0, "", 0);
	printf("-------------------------------------------------\n");
}

int Player::getIndex(int &index, int type)
{
	index = -1;
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == type)
		{
			index = i;
		}
		if (index != -1)
		{
			break;
		}
	}

	if (index == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}
}

int Player::openCodeCtx(AVCodecContext	*&pCodeCtx, int index)
{
	//�ҵ���Ӧ��type���ڵ�pFormatCtx->streams������λ��
	//��ȡ������
	pCodeCtx = pFormatCtx->streams[index]->codec;

	AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
	if (pCodec == NULL){
		printf("Codec not found.\n");
		return -1;
	}

	//�򿪽�����
	if (avcodec_open2(pCodeCtx, pCodec, NULL)<0)
	{
		printf("Could not open codec.\n");
		return -1;
	}
	return 0;
}

int Player::setWindow(AVCodecContext *pCodeCtx, const void *data)
{
	SDL_Window *screen = SDL_CreateWindowFrom(data);

	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodeCtx->width, pCodeCtx->height);
	//ͼ��ɫ�ʿռ�ת��
	img_convert_ctx = sws_getContext(pCodeCtx->width, pCodeCtx->height, pCodeCtx->pix_fmt,
		pCodeCtx->width, pCodeCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
}

int Player::setWindow(AVCodecContext *pCodeCtx)
{
	int screen_w = 800;
	int screen_h = 480;
	SDL_Window *screen = SDL_CreateWindow("FFmpeg player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	screen_w, screen_h, SDL_WINDOW_RESIZABLE);

	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodeCtx->width, pCodeCtx->height);
	//ͼ��ɫ�ʿռ�ת��
	img_convert_ctx = sws_getContext(pCodeCtx->width, pCodeCtx->height, pCodeCtx->pix_fmt,
		pCodeCtx->width, pCodeCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
}

void Player::audioSetting(AVCodecContext *pCodeCtx)
{
	//�ز�������ѡ��-----------------------------------------------------------start
	//����Ĳ�����ʽ
	enum AVSampleFormat in_sample_fmt = pCodeCtx->sample_fmt;
	//����Ĳ�����ʽ 16bit PCM
	out_sample_fmt = AV_SAMPLE_FMT_S16;
	//����Ĳ�����
	int in_sample_rate = pCodeCtx->sample_rate;
	//����Ĳ�����
	int out_sample_rate = 44100;
	//�������������
	uint64_t in_ch_layout = pCodeCtx->channel_layout;
	if (in_ch_layout <= 0)
	{
		in_ch_layout = av_get_default_channel_layout(pCodeCtx->channels);
	}
	//�������������
	uint64_t out_ch_layout = AV_CH_LAYOUT_MONO;

	//frame->16bit 44100 PCM ͳһ��Ƶ������ʽ�������
	
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
	wanted_spec.samples = pCodeCtx->frame_size;
	wanted_spec.callback = fill_audio;//�ص�����
	wanted_spec.userdata = this;
	if (SDL_OpenAudio(&wanted_spec, NULL)<0){
		printf("can't open audio.\n");
	}
	//�ط���Ƶ���� 
	SDL_PauseAudio(0);

	pReadThread = SDL_CreateThread(readThread, NULL, this);
	pDecodeThread = SDL_CreateThread(decodeThread, NULL, this);
}

int Player::readThread(void *opaque)
{
	Player *pPlayer = (Player *)opaque;
	for (;;)
		if (pPlayer->read() == 0)
			break;
	return 0;
}

int Player::decodeThread(void *opaque)
{
	Player *pPlayer = (Player *)opaque;
	for (;;)
		if (pPlayer->decode() == 0)
			break;
	return 0;
}

void  Player::fill_audio(void *udata, Uint8 *stream, int len)
{
	Player *pPlayer = (Player *)udata;
	pPlayer->audioCallback(stream, len);
}