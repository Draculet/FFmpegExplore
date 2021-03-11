#include <stdio.h>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
};
#include <arpa/inet.h> 


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
	char filepath[]="output2.mp4";
	FILE *fd = fopen("output.h264", "wb");
	FILE *fdx = fopen("outputx.yuv", "wb");
	FILE *fdy = fopen("outputy.yuv", "wb");

	int frame_cnt;

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
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
	av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");
	printf("pCodexCtx->pix_fmt: %d\n", pCodecCtx->pix_fmt);
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV411P, SWS_BICUBIC, NULL, NULL, NULL); 

	frame_cnt=0;

/*	AVBSFContext * h264bsfc;
	const AVBitStreamFilter * filter = av_bsf_get_by_name("h264_mp4toannexb");
	av_bsf_alloc(filter, &h264bsfc);
	avcodec_parameters_copy(h264bsfc->par_in, pFormatCtx->streams[videoindex]->codecpar);
	av_bsf_init(h264bsfc);
	for (int i = 0; i < pCodecCtx->extradata_size; i++){
		printf("%0x ", pCodecCtx->extradata[i]);
	}
	printf("sps pps end\n");
*/
	while(av_read_frame(pFormatCtx, packet)>=0){
		//AVPacket packet_copy;
		//av_copy_packet(&packet_copy, packet);
		if(packet->stream_index==videoindex){
				/*
				 * 在此处添加输出H264码流的代码
				 * 取自于packet，使用fwrite()
				 */
			//AVPacket pack = *packet;
			/*for (int i = 0; i < 100; i++){
				printf("%0x ", packet->data[i]);
			}
			printf("raw packet->size: %d\n", packet->size);
			//printf("raw code\n");
			int32_t numsize = 0;
			memcpy(&numsize, packet->data, 4);
			printf("test top 4 bytes %d\n", ntohl(numsize));
			ret = av_bsf_send_packet(h264bsfc, packet);
        	if(ret < 0) {
				printf("av_bsf_send_packet failed\n");
			}
			while ((ret = av_bsf_receive_packet(h264bsfc, packet)) == 0) {
				for (int i = 0; i < 100; i++){
					printf("%0x ", packet->data[i]);
				}
				printf("after bsf packet->size: %d\n", packet->size);
				printf("\n");
				fwrite(packet->data, packet->size, 1, fd);
			}
			//av_packet_unref(&pack);

			//fwrite(packet->data, 1, packet->size, fd);
			
			//ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            //new API 解决缓存帧问题
            */
            
/*			
			ret = avcodec_send_packet(pCodecCtx, packet);
			if ( ret == AVERROR(EAGAIN) || ret == AVERROR_EOF ) break;
        	else if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
            //printf("avcodec_send_packet\n");
			while (avcodec_receive_frame(pCodecCtx, pFrame) == 0){
				//printf("pixel_format: %d\n", pFrame->format);
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
					pFrameYUV->data, pFrameYUV->linesize);
				//printf("pixel_format: %d\n", pFrameYUV->format);
				printf("Decoded frame index: %d\n",frame_cnt);
				fwrite(pFrameYUV->data[0], 1, pCodecCtx->height * pCodecCtx->width, fdx);
				fwrite(pFrameYUV->data[1], 1, pCodecCtx->height * pCodecCtx->width, fdx);
				fwrite(pFrameYUV->data[2], 1, pCodecCtx->height * pCodecCtx->width, fdx);
				/*
				 * 在此处添加输出YUV的代码
				 * 取自于pFrameYUV，使用fwrite()
				 */
//				frame_cnt++;
//			}
			/*
				packet_copy,注意如果带缓存则大概率出错
			*/
			ret = avcodec_send_packet(pCodecCtx, packet);
        	if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
            //printf("avcodec_send_packet\n");
			while (ret = avcodec_receive_frame(pCodecCtx, pFrame) >= 0){
                if ( ret == AVERROR(EAGAIN) || ret == AVERROR_EOF ) break;
                else if(ret < 0){
                    printf("Decode Error.\n");
                    return -1;
                }
				//printf("pixel_format: %d\n", pFrame->format);
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
					pFrameYUV->data, pFrameYUV->linesize);
				//printf("pixel_format: %d\n", pFrameYUV->format);
				printf("Decoded frame index: %d\n",frame_cnt);
				fwrite(pFrameYUV->data[0], 1, pCodecCtx->height * pCodecCtx->width, fdy);
				fwrite(pFrameYUV->data[1], 1, pCodecCtx->height * pCodecCtx->width/4, fdy);
				fwrite(pFrameYUV->data[2], 1, pCodecCtx->height * pCodecCtx->width/4, fdy);
				/*
				 * 在此处添加输出YUV的代码
				 * 取自于pFrameYUV，使用fwrite()
				 */
				frame_cnt++;
			}
		}
		//av_packet_unref(packet);
		av_free_packet(packet);
		//av_free_packet(&packet_copy);
	}

	sws_freeContext(img_convert_ctx);

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	fclose(fd);

	return 0;
}

