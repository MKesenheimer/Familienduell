//g++ textediting.cpp -o test -F/Library/Frameworks/ -framework SDL2 -framework SDL2_ttf

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")
#pragma comment(lib, "SDL2_ttf.lib")

#include <string>
#include <iostream>

using namespace std;

const int SCREEN_WIDTH  = 1200;
const int SCREEN_HEIGHT = 800;
const int FULL_SCREEN_WIDTH = 1400;
const int FULL_SCREEN_HEIGHT = 900;
const int NUMBER_OF_LINES = 15;
const int NUMBER_OF_COLUMNS = 20;
const int FRAMES_PER_SECOND = 20;

//toggle full screen
void toggleFullscreen(SDL_Window *window, SDL_Renderer *renderer) {
        Uint32 flags(SDL_GetWindowFlags(window));
        flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_SetWindowFullscreen(window, flags);
        if((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
            SDL_RenderSetLogicalSize(renderer, FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT);
        } else {
            SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
        }
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int fontSize, SDL_Renderer *renderer) {
	//Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr) {
		TTF_CloseFont(font);
		return nullptr;
	}
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
	SDL_Surface *surf = TTF_RenderUTF8_Blended(font, message.c_str(), color);
	if (surf == nullptr) {
		TTF_CloseFont(font);
		return nullptr;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr) {
		SDL_FreeSurface(surf);
		TTF_CloseFont(font);
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}

int main(int argc, char *argv[]) {

    string text = " ";
	
	int SDL_Init(SDL_INIT_EVERYTHING);

	if (TTF_Init() != 0) {
		SDL_Quit();
		return 1;
	}

    bool quit = false;
	
    SDL_Window    *window = NULL;
    SDL_Renderer  *renderer = NULL;

    window = SDL_CreateWindow("Text input showing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 600, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		
    while (!quit) {
        SDL_StartTextInput();
        SDL_Event event;
        
        //get the current display mode
        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);
        
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDLK_ESCAPE:
                toggleFullscreen(window, renderer);
                break;
            case SDL_TEXTINPUT:
                //Add new text onto the end of our text
                text.append(event.text.text);
                break;
            case SDL_TEXTEDITING:
                //Update the composition text.
                //Update the cursor position.
                //Update the selection length (if any).
                //text.append(event.edit.text);
                break;
            }

            SDL_Color color = { 255, 255, 255, 255 };
            SDL_Texture *image = renderText(text.c_str(), "../monaco.ttf", color, current.h/NUMBER_OF_LINES, renderer);
            if (image == nullptr){
                TTF_Quit();
                SDL_Quit();
                return 1;
            }
            int x = 0;
            int y = 10;
            
            SDL_RenderClear(renderer);
            //We can draw our message as we do any other texture, since it's been
            //rendered to a texture
            renderTexture(image, renderer, x, y,text.size()*12,24);
            SDL_RenderPresent(renderer);
        }

        SDL_StopTextInput();
    }

    SDL_Quit();
	return 0;
}

