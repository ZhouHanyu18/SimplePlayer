#include "VideoPlayer.h"

VideoPlayer::VideoPlayer()
{
	bStart = FALSE;
	bStop = FALSE;
	finishRead = FALSE; 
	bQuit = FALSE;
	dSpeed = 1;
	qAudio = new PacketQueue();
	qVideo = new PacketQueue();

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	packetVideo = (AVPacket *)av_malloc(sizeof(AVPacket));
	pVideoFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	audioBuffer = (uint8_t *)av_malloc(2*44100);
	packetAudio = (AVPacket *)av_malloc(sizeof(AVPacket));
	pAudioFrame = av_frame_alloc();
	
}

VideoPlayer::~VideoPlayer()
{
	av_free_packet(packetAudio);
	av_free(audioBuffer);
	av_frame_free(&pAudioFrame);

	av_free(videoBuffer);
	av_frame_free(&pFrameYUV);
	av_frame_free(&pVideoFrame);

	av_free_packet(packet);
	delete qVideo;
	delete qAudio;
}

int VideoPlayer::read()
{
	if (bStart && !bStop)
	{
		if (dSpeed>0.001)
		{
			//av_seek_frame(pFormatCtx, indexAudio, (dSpeed + ptsAudio / 1000) / av_q2d(pAudioCodeCtx->time_base), AVSEEK_FLAG_FRAME);
		}
		//一帧一帧读取压缩的音频数据AVPacket
		if (av_read_frame(pFormatCtx, packet) >= 0)
		{
			if (packet->stream_index == indexAudio)
			{
				qAudio->pushQueue(packet);
			}
			else if (packet->stream_index == indexVideo)
			{
				qVideo->pushQueue(packet);
			}
		}
		else
		{
			finishRead = TRUE;
			return 0;
		}
	}
	return -1;
}

//设置音频数据长度
static Uint32 audio_len;
static Uint8 *audio_pos;
void VideoPlayer::audioCallback(Uint8 *stream, int len)
{
	if (bStart && !bStop)
	{
		SDL_memset(stream, 0, len);
		if (audio_len == 0)		//有数据才播放
			return;
		len = (len>audio_len ? audio_len : len);

		SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
		audio_pos += len;
		audio_len -= len;
	}
}

int VideoPlayer::decode()
{
	if (bStart && !bStop)
	{
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		return decodeAudio();
	}
	return -1;
}

int VideoPlayer::play(char *filepath, const void *handle)
{
	if (openFile(filepath) == -1)
	{
		return -1;
	}
	time = pFormatCtx->duration / 1000;
	if (getIndex(indexAudio, AVMEDIA_TYPE_AUDIO) == -1)
	{
		return -1;
	}
	if (getIndex(indexVideo, AVMEDIA_TYPE_VIDEO) == -1)
	{
		return -1;
	}
	if (openCodeCtx(pAudioCodeCtx, indexAudio) == -1)
	{
		return -1;
	}
	if (openCodeCtx(pVideoCodeCtx, indexVideo) == -1)
	{
		return -1;
	}
	if (handle)
		setWindow(pVideoCodeCtx, handle);
	else
		setWindow(pVideoCodeCtx);
	videoBuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pVideoCodeCtx->width, pVideoCodeCtx->height, 1));
	audioSetting(pAudioCodeCtx);
	bStart = TRUE;

	for (;;)
	{
		if (bQuit)
			break;
		if (decodeVideo() == 0)
			break;
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT)
		{
			if (ptsVideo > ptsAudio)
			{
				if (ptsVideo - ptsAudio < 1000)//跳过错误帧
					SDL_Delay(ptsVideo - ptsAudio);
			}
			//SDL---------------------------
			SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
			SDL_RenderPresent(sdlRenderer);
			//SDL End-----------------------
		}
		
	}
}

int VideoPlayer::decodeAudio()
{
	if (finishRead && qAudio->size == 0)
	{
		printf("outqVideo\n");
		return 0;//表示已完成
	}
	if (qAudio->getQueue(packetAudio))
	{
		int ret, got_frame, framecount = 0;
			
		ret = avcodec_decode_audio4(pAudioCodeCtx, pAudioFrame, &got_frame, packetAudio);
		ptsAudio = av_q2d(pFormatCtx->streams[indexAudio]->time_base) * packetAudio->pts * 1000;
		if (ret < 0) {
			printf("%s", "解码完成");
		}
		int out_buffer_size;
		if (got_frame) {
			swr_convert(swrCtx, &audioBuffer, 2 * 44100, (const uint8_t **)pAudioFrame->data, pAudioFrame->nb_samples);
			//获取sample的size
			out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb, pAudioFrame->nb_samples,
				out_sample_fmt, 1);
			//设置音频数据长度
			for (;;)
			{
				if (audio_len == 0)
				{
					audio_pos = (Uint8 *)audioBuffer;
					audio_len = out_buffer_size;
					break;
				}
			}
		}
		av_free_packet(packetAudio);
	}
	return -1;//表示未完成
}

int VideoPlayer::decodeVideo()
{
	if (bStop)
	{
		return -1;
	}
	if (finishRead && qVideo->size == 0)
	{
		printf("outqVideo\n");
		return 0;//表示已完成
	}
	if (qVideo->getQueue(packetVideo))
	{
		int ret, got_picture;
		ret = avcodec_decode_video2(pVideoCodeCtx, pVideoFrame, &got_picture, packetVideo);
		if (packetVideo->dts != AV_NOPTS_VALUE)
		{
			ptsVideo = av_frame_get_best_effort_timestamp(pVideoFrame)*av_q2d(pFormatCtx->streams[indexVideo]->time_base) * 1000;
		}
		else
		{
			ptsVideo = 0;
		}
		if (ret < 0){
			printf("Decode Error.\n");
		}
		if (got_picture)
		{
			//初始化缓冲区
			av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, videoBuffer,
				AV_PIX_FMT_YUV420P, pVideoCodeCtx->width, pVideoCodeCtx->height, 1);
			sws_scale(img_convert_ctx, (const unsigned char* const*)pVideoFrame->data, pVideoFrame->linesize, 0, pVideoCodeCtx->height, pFrameYUV->data, pFrameYUV->linesize);
		}
		av_free_packet(packetVideo);
	}
	return -1;//表示未完成
}

void VideoPlayer::stop()
{
	bStop = !bStop;
	if (bStop)
	{
		SDL_PauseAudio(1);
	}
	else
	{
		SDL_PauseAudio(0);
	}
}

void VideoPlayer::seek(double T)
{
	bStop = TRUE;
	SDL_PauseAudio(1);
	qAudio->initQueue();
	qVideo->initQueue();
	av_seek_frame(pFormatCtx, indexAudio, (T + ptsAudio / 1000) / av_q2d(pAudioCodeCtx->time_base), AVSEEK_FLAG_BACKWARD);
	bStop = FALSE;
	SDL_PauseAudio(0);
}

void VideoPlayer::speed(double S)
{
	dSpeed = S;
}
