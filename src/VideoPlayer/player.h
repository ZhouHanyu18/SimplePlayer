#ifndef PLAYER_H
#define PLAYER_H
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

extern "C"
{
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
//播放
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
	void initQueue();
};
class Player
{
public:
	Player();
	~Player();
public:
	AVFormatContext	*pFormatCtx;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	struct SwsContext *img_convert_ctx;
	int out_channel_nb;
	enum AVSampleFormat out_sample_fmt;
	SwrContext *swrCtx;
public:
	int openFile(char *filepath);
	void showInfo();
	int getIndex(int &index, int type);
	int openCodeCtx(AVCodecContext	*&pCodeCtx, int index);
	int setWindow(AVCodecContext *pCodeCtx, const void *data);
	int setWindow(AVCodecContext *pCodeCtx);
	void audioSetting(AVCodecContext *pCodeCtx);
	SDL_Thread *pReadThread;
	SDL_Thread *pDecodeThread;
	static int readThread(void *opaque);
	static int decodeThread(void *opaque);
	static void fill_audio(void *udata, Uint8 *stream, int len);
	virtual int read()=0;
	virtual int decode()=0;
	virtual void audioCallback(Uint8 *stream, int len)=0;
	
};
#endif	//PLAYER_H