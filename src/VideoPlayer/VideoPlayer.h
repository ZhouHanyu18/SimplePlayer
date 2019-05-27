#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
#define TRUE 1
#define FALSE 0

#include "player.h"
class VideoPlayer:public Player
{
public:
	VideoPlayer();
	~VideoPlayer();
private:
	bool finishRead;//�������
	bool bStop;//��ͣ����
	bool bStart;//��ʼ����

	SDL_Event event;
	int indexVideo;
	int indexAudio;
	AVPacket *packet;
	AVCodecContext	*pVideoCodeCtx;
	AVCodecContext	*pAudioCodeCtx;

	PacketQueue *qVideo;
	PacketQueue *qAudio;
	double ptsAudio;
	double ptsVideo;

	AVPacket *packetVideo;
	AVFrame	*pVideoFrame;
	AVFrame	*pFrameYUV;
	unsigned char *videoBuffer;

	uint8_t *audioBuffer;
	AVPacket *packetAudio;
	AVFrame *pAudioFrame;

public:
	int play(char *filepath, const void *handle);
	void seek(double T);
private:
	int decodeAudio();
	int decodeVideo();
	virtual int read();
	virtual int decode();
	virtual void audioCallback(Uint8 *stream, int len);
};
#endif