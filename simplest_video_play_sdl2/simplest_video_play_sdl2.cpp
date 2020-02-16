/**
 * 最简单的SDL2播放视频的例子（SDL2播放RGB/YUV）
 * Simplest Video Play SDL2 (SDL2 play RGB/YUV) 
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序使用SDL2播放RGB/YUV视频像素数据。SDL实际上是对底层绘图
 * API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层
 * API。
 *
 * 函数调用步骤如下: 
 *
 * [初始化]
 * SDL_Init(): 初始化SDL。
 * SDL_CreateWindow(): 创建窗口（Window）。
 * SDL_CreateRenderer(): 基于窗口创建渲染器（Render）。
 * SDL_CreateTexture(): 创建纹理（Texture）。
 *
 * [循环渲染数据]
 * SDL_UpdateTexture(): 设置纹理的数据。
 * SDL_RenderCopy(): 纹理复制给渲染器。
 * SDL_RenderPresent(): 显示。
 *
 * This software plays RGB/YUV raw video data using SDL2.
 * SDL is a wrapper of low-level API (Direct3D, OpenGL).
 * Use SDL is much easier than directly call these low-level API.  
 *
 * The process is shown as follows:
 *
 * [Init]
 * SDL_Init(): Init SDL.
 * SDL_CreateWindow(): Create a Window.
 * SDL_CreateRenderer(): Create a Render.
 * SDL_CreateTexture(): Create a Texture.
 *
 * [Loop to Render data]
 * SDL_UpdateTexture(): Set Texture's data.
 * SDL_RenderCopy(): Copy Texture to Render.
 * SDL_RenderPresent(): Show.
 */

#include <stdio.h>

extern "C"
{
#include "sdl/SDL.h"
};

const int bpp=12;

int screen_w=480,screen_h=272;
const int pixel_w=480,pixel_h=272;

unsigned char buffer1[pixel_w*pixel_h*bpp/8];
unsigned char buffer2[pixel_w*pixel_h*bpp/8];

unsigned char buffer[pixel_w*pixel_h*bpp/8];

//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)

#define BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;

int refresh_video(void *opaque){
	thread_exit=0;
	while (!thread_exit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	thread_exit=0;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;
	SDL_PushEvent(&event);

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
	screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  

	Uint32 pixformat=0;

	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat= SDL_PIXELFORMAT_IYUV;  

	SDL_Texture* sdlTexture1 = SDL_CreateTexture(sdlRenderer,pixformat, SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);
	SDL_Texture* sdlTexture2 = SDL_CreateTexture(sdlRenderer,pixformat, SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);


	FILE *fp1 = NULL;
	fp1 = fopen("bigbuckbunny_480x272.yuv","rb+");

	FILE *fp2 = NULL;
	fp2 = fopen("bigbuckbunny_blackwidth_480x272.yuv","rb+");

	if(fp1 == NULL || fp2 == NULL)
	{
		printf("cannot open files\n");
		return -1;
	}

	SDL_Rect sdlRect1;  
	SDL_Rect sdlRect2;  

	int left_width = 40;
	int index = 0;
	int inc = 1;

	SDL_Thread *refresh_thread = SDL_CreateThread(refresh_video,NULL,NULL);
	SDL_Event event;
	while(1){
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==REFRESH_EVENT){
			if (fread(buffer1, 1, pixel_w*pixel_h*bpp/8, fp1) != pixel_w*pixel_h*bpp/8){
				// Loop
				fseek(fp1, 0, SEEK_SET);
				fread(buffer1, 1, pixel_w*pixel_h*bpp/8, fp1);
			}

			if (fread(buffer2, 1, pixel_w*pixel_h*bpp/8, fp2) != pixel_w*pixel_h*bpp/8){
				// Loop
				fseek(fp2, 0, SEEK_SET);
				fread(buffer2, 1, pixel_w*pixel_h*bpp/8, fp2);
			}

			index = 0;

			for(int i = 0; i < pixel_h; ++i)
			{
				//fwrite(buffer1 + pixel_w * i, left_width, 1, buffer);
				memcpy(buffer + index, buffer1 + pixel_w * i, left_width);
				index += left_width;
				memcpy(buffer + index, buffer2 + pixel_w * i + left_width, pixel_w - left_width);
				index += (pixel_w - left_width);
			}

			for(int i  = 0; i < pixel_h / 2; ++i)
			{
				memcpy(buffer + index, buffer1 + pixel_h * pixel_w + pixel_w / 2 * i, left_width / 2);
				index += left_width / 2;
				memcpy(buffer + index, buffer2 + pixel_h * pixel_w + pixel_w / 2 * i + left_width / 2, pixel_w / 2 - left_width / 2);
				index += pixel_w / 2 - left_width / 2;
			}

			for(int i  = 0; i < pixel_h / 2; ++i)
			{
				memcpy(buffer + index, buffer1 + pixel_h * pixel_w * 5 / 4 + pixel_w / 2 * i, left_width / 2);
				index += left_width / 2;
				memcpy(buffer + index, buffer2 + pixel_h * pixel_w * 5 / 4 + pixel_w / 2 * i + left_width / 2, pixel_w / 2 - left_width / 2);
				index += pixel_w / 2 - left_width / 2;
			}

			for(int i = 0; i < pixel_h; ++i)
			{
//				buffer[pixel_w * i + left_width] = 0;
			}

			if(inc == 1)
			{
				if(left_width < 440)
				{
					left_width += 4;
				}
				else
				{
					inc = 0;
				}
			}
			else
			{
				if(left_width > 40)
				{
					left_width -= 4;
				}
				else
				{
					inc = 1;
				}				
			}

			SDL_UpdateTexture(sdlTexture1, NULL, buffer, pixel_w);  

//			SDL_UpdateTexture(sdlTexture2, NULL, buffer2, pixel_w); 

			//FIX: If window is resize
			sdlRect1.x = 0;  
			sdlRect1.y = 0;  
			sdlRect1.w = screen_w;  
			sdlRect1.h = screen_h;  

			SDL_RenderClear( sdlRenderer );   
			SDL_RenderCopy( sdlRenderer, sdlTexture1, NULL, &sdlRect1); 
			SDL_RenderPresent( sdlRenderer );  
			
		}else if(event.type==SDL_WINDOWEVENT){
			//If Resize
//			SDL_GetWindowSize(screen,&screen_w,&screen_h);
		}else if(event.type==SDL_QUIT){
			thread_exit=1;
		}else if(event.type==BREAK_EVENT){
			break;
		}
	}
	SDL_Quit();
	return 0;
}
