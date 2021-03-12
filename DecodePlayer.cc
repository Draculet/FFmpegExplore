#include <stdio.h>
#include <unistd.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <SDL2/SDL.h>
};
#include <arpa/inet.h> 


int screen_w = 800,screen_h = 600;
#define LOADPIC_EVENT  (SDL_USEREVENT + 1)
#define BREAK_EVENT  (SDL_USEREVENT + 2)

bool quit = false;
int threadfunc(void *opaque){
	while (!quit) {
		SDL_Event event;
		event.type = LOADPIC_EVENT;
		SDL_PushEvent(&event);
        SDL_Delay(40);
    }
	return 0;
}

int main(int argc, char* argv[])
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame,*pFrameYUV;
	uint8_t *out_buffer;
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;
	//输入文件路径
	char filepath[]="output2tmp.mp4";
	//FILE *fd = fopen(filepath, "wb");
    int frame_cnt;

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}

	double totalsec = pFormatCtx->duration * av_q2d(AV_TIME_BASE_Q);
	printf("video duration: %lf\n", totalsec);

	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex=-1;
	//AVStream
	printf("test nb_streams: %d\n", pFormatCtx->nb_streams);

	for (i=0; i < pFormatCtx->nb_streams; i++){
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
    }
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if( pCodec == NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}


    //SDL Init
    if(SDL_Init(SDL_INIT_VIDEO)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	}
	SDL_Window *screen; 
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("SDL2 player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen) {
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
    SDL_Rect sdlRect;
	SDL_Event event;
    pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
    out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	frame_cnt=0;
    uint8_t buf[pCodecCtx->height * pCodecCtx->width * 3 / 2] = {0};
    SDL_Thread *refresh_thread = SDL_CreateThread(threadfunc,NULL,NULL);
    while (true) {
		SDL_WaitEvent(&event);
		if(event.type == LOADPIC_EVENT){
			while (av_read_frame(pFormatCtx, packet) >= 0){
                if(packet->stream_index==videoindex){
                    ret = avcodec_send_packet(pCodecCtx, packet);
                    if(ret < 0){
                        printf("Decode Error.\n");
                        return -1;
                    }
                    while (avcodec_receive_frame(pCodecCtx, pFrame) == 0){
                        sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
                            pFrameYUV->data, pFrameYUV->linesize);
                        printf("Decoded frame index: %d\n",frame_cnt);
                        int h = pCodecCtx->height, w = pCodecCtx->width;
                        memcpy(buf, pFrame->data[0], h * w);
                        memcpy(buf + w * h, pFrame->data[1], h * w / 4);
                        memcpy(buf + w * h * 5 / 4, pFrame->data[2], h * w / 4);
                        frame_cnt++;
                        printf("present %dx%d test%d\n", h ,w, pFrame->linesize[0]);
                        SDL_UpdateTexture( sdlTexture, NULL, buf, w);
                        //FIX: If window is resize
                        sdlRect.x = screen_w/4;  
                        sdlRect.y = screen_h/4;
                        sdlRect.w = screen_w/2;
                        sdlRect.h = screen_h/2;
                        SDL_RenderClear( sdlRenderer );   
                        SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);  
                        SDL_RenderPresent( sdlRenderer ); 
                    }
                    av_free_packet(packet);
                    break;
                }
                av_free_packet(packet);
            }
		} else if(event.type == SDL_WINDOWEVENT){
			SDL_GetWindowSize(screen, &screen_w, &screen_h);
		} else if(event.type == SDL_QUIT){
			quit = true;
            printf("SDL_QUIT EVENT\n");
            break;
		}
	}
	SDL_Quit();


	sws_freeContext(img_convert_ctx);

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

