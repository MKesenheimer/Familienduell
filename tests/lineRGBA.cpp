//g++ -std=c++11 lineRGBA.cpp -o test -F/Library/Frameworks -framework SDL2 -framework SDL2_gfx && ./test

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <SDL2/SDL.h>
#include <SDL2_gfx/SDL2_gfxPrimitives.h>
#include <math.h>
#include "cleanup.h"

/* Notes:
 * passing values
 * void foo(vector<int> bar); // by value
 * void foo(vector<int> &bar); // by reference (non-const, so modifyable inside foo)
 * void foo(vector<int> const &bar); // by const-reference
 *
 * passing arrays
 * void foo(int bar[2]); // by value
 * void foo(int (&bar)[2]); // by reference (non-const, so modifyable inside foo)
 * void foo(int const (&bar)[2]); // by const-reference
 */

using namespace::std;

const int SCREEN_WIDTH  = 1200;
const int SCREEN_HEIGHT = 800;

//Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
//width and height
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

//Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
//the texture's width and height
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture(tex, ren, x, y, w, h);
}

void renderSurface(SDL_Renderer *ren, SDL_Surface *surf, int x, int y) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surf);
	if (texture == nullptr) {
        std::cout<<"Error in renderText."<<std::endl;
		SDL_FreeSurface(surf);
	}
    renderTexture(texture, ren, x, y);
}

int main(int argc, char* args[]) {
    
    SDL_Texture *texture;
    SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("Familienduell", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); // | SDL_WINDOW_ALLOW_HIGHDPI
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    //For tracking if we want to quit
	bool quit = false;
    while(!quit){
        //Our event structure
        SDL_Event e;

		while(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }
        //Rendering
        SDL_RenderClear(renderer);
        //Draw the background white
        boxRGBA(renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 255, 255, 255, 255);
        
        //boxRGBA(renderer, 0, 0, 10, 10, 172, 200, 0, 255);

        //SDL_SetRenderDrawColor(renderer, 172, 200, 0, 255);
        //SDL_RenderDrawLine(renderer, 0, 0, 30, 0);
        lineRGBA(renderer, 0, 0, 30, 0, 172, 200, 0, 255);

        //apply to the screen
        SDL_RenderPresent(renderer);
    }

    //destroy the items
	cleanup(renderer, window);
	SDL_Quit();
	return 0;
}