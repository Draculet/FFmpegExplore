#include <stdio.h>
#include <malloc.h>

int main(void){
    FILE *fp_yuv = fopen("outputyy.yuv", "rb+");
    FILE *fp_y = fopen("output_y.y", "wb+");
    FILE *fp_u = fopen("output_u.y", "wb+");
    FILE *fp_v = fopen("output_v.y", "wb+");
    int width = 1960, height = 1080;
    unsigned char *data = (unsigned char *) malloc(width * height * 3);

    fread(data, 1, width * height * 3, fp_yuv);
    //Y
    fwrite(data, 1, width * height, fp_y);
    for (int i = 0; i < width * height * 2; i++){
        if (i % 2 == 0)
            fwrite(data + i, 1, 1, fp_u);
        else
            fwrite(data + i, 1, 1, fp_v);
    }

    //释放资源
    free(data);

    fclose(fp_yuv);
    fclose(fp_y);
    fclose(fp_u);
    fclose(fp_v);
    fp_yuv = fopen("outputff.yuv", "rb+");

    fp_y = fopen("output_420_y.y", "wb+");
    fp_u = fopen("output_420_u.y", "wb+");
    fp_v = fopen("output_420_v.y", "wb+");

    data = (unsigned char *) malloc(width * height * 3 / 2);

    fread(data, 1, width * height * 3 / 2, fp_yuv);
    //Y
    fwrite(data, 1, width * height, fp_y);
    //U
    fwrite(data + width * height, 1, width * height / 4, fp_u);
    //V
    fwrite(data + width * height * 5 / 4, 1, width * height / 4, fp_v);

    //释放资源
    free(data);

    fclose(fp_yuv);
    fclose(fp_y);
    fclose(fp_u);
    fclose(fp_v);
}
