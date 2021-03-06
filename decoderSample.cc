#include <stdio.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
};
#include <arpa/inet.h>
#include <string>


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
	//char filepath[]="/home/ubuntu/video_example/";
	std::string filepath = "/home/ubuntu/video_example/" + std::string(argv[1]);
	int frame_cnt;
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, filepath.c_str(), NULL, NULL) != 0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex=-1;
	//AVStream
	printf("test nb_streams: %d\n", pFormatCtx->nb_streams);

	for (i=0; i<pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}

    //new API
	//pCodecCtx=pFormatCtx->streams[videoindex]->codec;
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}
	/*
	 * 在此处添加输出视频信息的代码
	 * 取自于pFormatCtx，使用fprintf()
	 */
	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
	out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV411P, pCodecCtx->width, pCodecCtx->height));
	//avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV411P, pCodecCtx->width, pCodecCtx->height, 1);
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx,0,filepath.c_str(),0);
	printf("-------------------------------------------------\n");
	printf("pCodexCtx->pix_fmt: %d\n", pCodecCtx->pix_fmt);
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV411P, SWS_BICUBIC, NULL, NULL, NULL); 

	frame_cnt=0;
	printf("total time: %lf\n", pFormatCtx->duration * av_q2d(AV_TIME_BASE_Q));
	
	while(av_read_frame(pFormatCtx, packet)>=0){
		if(packet->stream_index==videoindex){
			printf("packet PTS:%ld DTS:%ld\n", packet->pts, packet->dts);
			if (packet->flags & AV_PKT_FLAG_KEY){
				//printf("packet is key frame\n");
			}
			/*
			printf("packet dts:%ld pts:%ld\n", packet->dts, packet->pts);
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			if(got_picture){
				printf("got pic\n");
			*/
			//printf("packet is ", packet-> )
			ret = avcodec_send_packet(pCodecCtx, packet);
        	if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
            printf("avcodec_send_packet\n");
			
			/*
			printf("AVStream timebase: %d/%d AVCodec timebase: %d/%d, ticks: %d, fps: %lf\n",
				pFormatCtx->streams[videoindex]->time_base.den,
					pFormatCtx->streams[videoindex]->time_base.num,
						pCodecCtx->time_base.den,
							pCodecCtx->time_base.num,
								pCodecCtx->ticks_per_frame,
									(double)pCodecCtx->time_base.den / pCodecCtx->time_base.num / pCodecCtx->ticks_per_frame);
			exit(0);
			*/
			while (ret >= 0) {
				ret = avcodec_receive_frame(pCodecCtx, pFrame);
				//printf("recv ret %d\n", ret);
                if ( ret == AVERROR(EAGAIN) || ret == AVERROR_EOF ) {
					//printf("Eagain or eof\n");
					if (ret == AVERROR(EAGAIN)) printf("EAGAIN\n");
					else if (ret == AVERROR_EOF) printf("EOF\n");
					break;
				}
                else if(ret < 0){
                    printf("Decode Error.\n");
                    return -1;
                }
				printf("frame type: %d\n", pFrame->pict_type);
				printf("frame PTS:%ld DTS:%ld\n", pFrame->pkt_pts, pFrame->pkt_dts);
				printf("timestamp: %lf\n", pFrame->pkt_pts * av_q2d(pFormatCtx->streams[videoindex]->time_base));
				//printf("pixel_format: %d\n", pFrame->format);
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
					pFrameYUV->data, pFrameYUV->linesize);
				//printf("pixel_format: %d\n", pFrameYUV->format);
				printf("Decoded frame index: %d\n",frame_cnt);
				//fwrite(pFrameYUV->data[0], 1, pCodecCtx->height * pCodecCtx->width, fdy);
				//fwrite(pFrameYUV->data[1], 1, pCodecCtx->height * pCodecCtx->width/4, fdy);
				//fwrite(pFrameYUV->data[2], 1, pCodecCtx->height * pCodecCtx->width/4, fdy);
				frame_cnt++;
			}
		}
		av_free_packet(packet);
	}

	sws_freeContext(img_convert_ctx);

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

