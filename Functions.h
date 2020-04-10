#ifndef Functions_H
#define Functions_H

#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>
#include "SDL_Defs.h"

void strToChar(const std::string tmp, char (&chars)[1024]);
bool is_number(const std::string& s);
std::vector<std::string> strToVec(std::string str);
std::string readNextString(std::string str, std::size_t *pos);
void toggleFullscreen(SDL_Window *window, SDL_Renderer *renderer);
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h);
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y);
void renderSurface(SDL_Renderer *ren, SDL_Surface *surf, int x, int y);
void renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int alpha, int fontSize, SDL_Renderer *renderer, int x, int y);
void renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int fontSize, SDL_Renderer *renderer, int x, int y);

#endif
