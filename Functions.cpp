#include "Functions.h"
#include <algorithm>

void strToChar(const std::string tmp, char (&chars)[1024]) {
    strncpy(chars, tmp.c_str(), sizeof(chars));
    chars[sizeof(chars) - 1] = 0;
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

std::vector<std::string> strToVec(std::string str) {
    //extract the entries (ignore whitespaces)
    std::vector<std::string> entry;
    std::string strcopy = str;
    std::size_t pos = 0;
    while(pos!=std::string::npos) {
        std::string temp = readNextString(strcopy, &pos);
        //delete this string in blockContent[i]
        if(temp!="") {
            strcopy = strcopy.substr(pos+temp.length());
            entry.push_back(temp);
        }
    }
    return entry;
}

std::string readNextString(std::string str, std::size_t *pos) {
    std::string temp = std::string();
    *pos = std::string::npos;
    if(str!="" or str!=std::string()) {
        std::size_t n = str.find_first_not_of(" ");
        if(n!=std::string::npos) {
            std::size_t m = str.find_first_of(" ", n);
            temp = str.substr(n, m!=std::string::npos ? m-n : m);
            *pos = n;
        }
    }
    return temp;
}

//toggle full screen
void toggleFullscreen(SDL_Window *window, SDL_Renderer *renderer) {
        Uint32 flags(SDL_GetWindowFlags(window));
        flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_SetWindowFullscreen(window, flags);
        if((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
            SDL_RenderSetLogicalSize(renderer, FULLSCREEN_WIDTH, FULLSCREEN_HEIGHT);
        } else {
            SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
            SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        }
}

//Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
//width and height
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h) {
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
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
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
    SDL_DestroyTexture(texture);
}

//Render text
void renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int alpha, int fontSize, SDL_Renderer *renderer, int x, int y) {

	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if(font == nullptr) {
        std::cout<<"Error 1 in renderText.";
    }
    
	SDL_Surface *surf = TTF_RenderUTF8_Blended(font, message.c_str(), color);
    if(surf == nullptr) {
        std::cout<<"Error 2 in renderText.";
    }
    
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if(texture == nullptr) {
        std::cout<<"Error 3 in renderText.";
    }
    
    SDL_SetTextureAlphaMod(texture, alpha);
    renderTexture(texture, renderer, x, y);
    
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
}

//Render text (without alpha blending)
void renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int fontSize, SDL_Renderer *renderer, int x, int y) {

	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if(font == nullptr) {
        std::cout<<"Error 1 in renderText.";
    }
    
	SDL_Surface *surf = TTF_RenderUTF8_Blended(font, message.c_str(), color);
    if(surf == nullptr) {
        std::cout<<"Error 2 in renderText.";
    }
    
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if(texture == nullptr) {
        std::cout<<"Error 3 in renderText.";
    }
    
    renderTexture(texture, renderer, x, y);
    
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
}