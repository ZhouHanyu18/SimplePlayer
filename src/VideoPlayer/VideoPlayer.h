#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

extern "C"
{
	//��װ��ʽ
#include "libavformat/avformat.h"
	//����
#include "libavcodec/avcodec.h"
	//����
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
	//����
#include "SDL2/SDL.h"
};
struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int capacity;
	SDL_mutex *mutex;
	SDL_cond *cond;
	SDL_cond *full;
	PacketQueue();
	int pushQueue(AVPacket *pkt);
	int getQueue(AVPacket *pkt);
};
class VideoPlayer
{
public:
	VideoPlayer();
	~VideoPlayer();
private:
	AVFormatContext	*pFormatCtx;
	bool finishRead;

	int indexVideo;
	AVCodecContext	*pVideoCodeCtx;
	PacketQueue *qVideo;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Event event;
	unsigned char *videoBuffer;
	struct SwsContext *img_convert_ctx;
private:
	int indexAudio;
	int out_channel_nb;
	double ptsAudio;
	enum AVSampleFormat out_sample_fmt;
	AVCodecContext	*pAudioCodeCtx;
	PacketQueue *qAudio;
	SwrContext *swrCtx;
	uint8_t *audioBuffer;
	AVPacket *packetAudio;
	AVFrame *frame;
public:
	void audioSetting();
	int decodeAudio();
	static void fill_audio(void *udata, Uint8 *stream, int len);
public:
	int openFile(char *filepath);
	void showInfo();
	int setWindow(const void *data);
	int decodeVideo();
	virtual int play();
	static int sfp_refresh_thread(void *opaque);
	static int read_thread(void *opaque);
};
#endif