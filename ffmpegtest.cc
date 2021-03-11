extern "C"{
    #include <libavcodec/avcodec.h>
}

int main(void){
    //unsigned char i = 0b10000000;
    //printf("%d\n", i);
    printf("%s\n", avcodec_configuration());
}