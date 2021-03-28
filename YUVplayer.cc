#include <stdio.h>

extern "C"
{
#include <SDL2/SDL.h>
};


int screen_w = 800,screen_h = 600;
const int pixel_w = 1920, pixel_h = 1080;

unsigned char buffer[pixel_w * pixel_h * 3] = {0};// 4:4:4


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

	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

	FILE *fp=NULL;
	fp=fopen("output.rgb","rb");

	if(fp==NULL){
		printf("cannot open this file\n");
		return -1;
	}

	SDL_Rect sdlRect;  

	SDL_Thread *refresh_thread = SDL_CreateThread(threadfunc,NULL,NULL);
	SDL_Event event;
	while (true) {
		SDL_WaitEvent(&event);
		if(event.type == LOADPIC_EVENT){
			if (fread(buffer, 1, pixel_w * pixel_h * 3, fp) != pixel_w * pixel_h * 3){
				// Loop
				fseek(fp, 0, SEEK_SET);
				fread(buffer, 1, pixel_w*pixel_h*3, fp);
			}

			SDL_UpdateTexture( sdlTexture, NULL, buffer, pixel_w * 3);  

			//FIX: If window is resize
			sdlRect.x = screen_w/4;  
			sdlRect.y = screen_h/4;
			sdlRect.w = screen_w/2;
			sdlRect.h = screen_h/2;
			
			SDL_RenderClear( sdlRenderer );   
			SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);  
			SDL_RenderPresent( sdlRenderer );  
		} else if(event.type == SDL_WINDOWEVENT){
			SDL_GetWindowSize(screen, &screen_w, &screen_h);
		} else if(event.type == SDL_QUIT){
			quit = true;
            printf("SDL_QUIT EVENT\n");
            break;
		}
	}
	SDL_Quit();
	return 0;
}
