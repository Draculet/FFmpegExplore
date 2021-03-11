#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
	//读取文件test_yuv420p_320x180.yuv
	FILE* fp_yuv=fopen("/home/ubuntu/video_example/1920x1080test.yum","rb");
	//写入文件frame.yuv
	FILE* fp_frame=fopen("1920x1080out.yuv","wb");
	//开辟内存读取test_yuv420p_320x180.yuv文件的第一帧的亮度数据（Y）
	char* buffer_y=(char*)malloc(sizeof(char)*1920*1080);
	char* buffer_u=(char*)malloc(sizeof(char)*1920*1080/4);
	char* buffer_v=(char*)malloc(sizeof(char)*1920*1080/4);

	//读取函数，将test_yuv420p_320x180.yuv的第一帧存入buff指向的内存
	for(int i=0;i<100;i++)
	{
		fread(buffer_y,1920*1080,1,fp_yuv);
		fread(buffer_u,1920*1080/4,1,fp_yuv);
		fread(buffer_v,1920*1080/4,1,fp_yuv);

		//for(int i=0;i<320*180/4;i++){
		//	buffer_u[i]=128;
		//	buffer_v[i]=128;
		//}

		memset(buffer_u,127,1920*1080/4);
		memset(buffer_v,127,1920*1080/4);

		fwrite(buffer_y,1920*1080,1,fp_frame);
		fwrite(buffer_u,1920*1080/4,1,fp_frame);
		fwrite(buffer_v,1920*1080/4,1,fp_frame);
	}
	//fread(buff,320*180,1,fp_yuv);
	//将buff指向的内存写入frame.yuv
	

	//fprintf演示
	//FILE* fp_demo=fopen("demo.txt","wb");
	//char hello[20]="GuangDianGong";
	//fprintf(fp_demo,"Hello World,%s",hello);
	//fclose(fp_demo);

	free(buffer_y);
	free(buffer_u);
	free(buffer_v);
	fclose(fp_yuv);
	fclose(fp_frame);
	

	return 0;
}
