#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_gfx/SDL2_gfxPrimitives.h>
#include "SDL2_ttf/SDL_ttf.h"
#include <math.h>
#include <ncurses.h>
#include "Interface.h"
#include "Functions.h"
#include "Timer.h"
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

const int TERMINAL_WIDTH = 400;
const int TERMINAL_HEIGHT = 300;
const int SCREEN_WIDTH  = 1200;
const int SCREEN_HEIGHT = 800;
const int FULL_SCREEN_WIDTH = 1400;
const int FULL_SCREEN_HEIGHT = 900;
const int NUMBER_OF_LINES = 15;
const int NUMBER_OF_COLUMNS = 20;
const int FRAMES_PER_SECOND = 30;

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

//Render text
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
        std::cout<<"Error in renderText."<<std::endl;
		SDL_FreeSurface(surf);
		TTF_CloseFont(font);
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}

/*void fadeTexture(SDL_Texture *texture, float elapsedTime) {
    // Check there is a texture
    if (texture) {
        // Set the alpha of the texture
        SDL_SetTextureAlphaMod(texture, alpha);
    }

    // Update the alpha value
    if (alpha < SDL_ALPHA_OPAQUE) {
        alphaCalc += FADE_SPEED * elapsedTime;
        alpha = alphaCalc;
    }

    // if alpha is above 255 clamp it
    if (alpha >= SDL_ALPHA_OPAQUE) {
        alpha = SDL_ALPHA_OPAQUE;
        alphaCalc = (float)SDL_ALPHA_OPAQUE;
    }
}*/

int main(int argc, char* args[]) {
    //load the questions and the answers
    Interface interface("Fragen.txt");
    
    SDL_Texture *texture;
    
    //The text that's going to be used
    std::vector<std::string> textTerminal;
    std::vector<std::string> history;
    textTerminal.push_back("fd> ");
    int currentLine = 0;
    int selectCommand = 0;
    string textMain = " ";
    SDL_Color colorGreen = { 173, 200, 0 };
    SDL_Color colorWhite = { 255, 255, 255 };
    //Initialize SDL_ttf
    if( TTF_Init() != 0){
        std::cout<<"Error in TTF_Init"<<std::endl;
        return 1;
    }
    
    //Timer zum festlegen der FPS
	Timer fps;
    Timer worldtime;
    worldtime.start();
    //number of frame
	int frame = 0;

	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		std::cout<<"Error in SDL_Init"<<std::endl;
		return 1;
	}
    
    //Gamelogic
    bool showSplash = true;
    bool showAnswers = false;
    int currentQuestion = 0;
    int numberOfQuestions = 2;
    
    //Setup our window and renderer, this is our screen for the public
    SDL_Window *windowTerminal = SDL_CreateWindow("Familienduell", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, TERMINAL_WIDTH, TERMINAL_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *windowMain = SDL_CreateWindow("Familienduell", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	if (windowMain == NULL || windowTerminal == NULL){
		std::cout<<"Error in CreateWindow"<<std::endl;
		SDL_Quit();
		return 1;
	}
    SDL_Renderer *rendererTerminal = SDL_CreateRenderer(windowTerminal, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Renderer *rendererMain = SDL_CreateRenderer(windowMain, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (rendererMain == NULL || rendererTerminal == NULL){
		std::cout<<"Error: CreateRenderer"<<std::endl;
		cleanup(windowMain);
		SDL_Quit();
		return 1;
	}
    SDL_RaiseWindow(windowTerminal);
    
    //For tracking if we want to quit
	bool quit = false;
    while(!quit){
        //start text input
        SDL_StartTextInput();
        //start the fps timer
        fps.start();
        //Our event structure
        SDL_Event e;
        
        //get the current display mode
        SDL_DisplayMode current;
        SDL_GetCurrentDisplayMode(0, &current);
        
		while(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                //SDL_RaiseWindow(windowTerminal);
                switch(e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    toggleFullscreen(windowMain, rendererMain);
                    SDL_RaiseWindow(windowTerminal);
                    break;
                case SDLK_RETURN:
                    textTerminal.push_back("fd> ");
                    //handle terminal commands
                    if(currentLine>=0) {
                        std::string temp = textTerminal[currentLine];
                        if(temp.length()>4) {
                            temp = temp.substr(4,temp.length());
                            history.push_back(temp);
                            selectCommand = history.size();
                        }
                        if(temp!="") {
                            if(temp.compare("fullscreen") == 0) {
                                toggleFullscreen(windowMain, rendererMain);
                                SDL_RaiseWindow(windowTerminal);
                            }
                            if(temp.compare("exit") == 0) {
                                quit = true;
                            }
                            if(temp.compare("next") == 0) {
                                currentQuestion++;
                                if(currentQuestion<0) currentQuestion = 0;
                            }
                            if(temp.compare("previous") == 0) {
                                currentQuestion--;
                                if(currentQuestion>=numberOfQuestions) currentQuestion = numberOfQuestions-1;
                            }
                            if(temp.compare("start") == 0) {
                                showSplash = false;
                                showAnswers = true;
                            }
                            if(temp.compare("restart") == 0) {
                                showSplash = true;
                                showAnswers = false;
                                currentQuestion = 0;
                            }
                        }
                    }
                    currentLine++;
                    break;
                case SDLK_UP:
                    selectCommand--;
                    if(history.size()>0 and selectCommand>=0) {
                        textTerminal[currentLine] = "fd> " + history[selectCommand];
                    } else {
                        selectCommand = 0;
                    }
                    break;
                case SDLK_DOWN:
                    selectCommand++;
                    if(selectCommand<=history.size()) {
                        textTerminal[currentLine] = "fd> " + history[selectCommand];
                    } else {
                        selectCommand = history.size();
                    }
                    break;
                case SDLK_BACKSPACE:
                    std::string temp = textTerminal[currentLine];
                    if(temp.length()>4) {
                        textTerminal[currentLine] = temp.substr(0,temp.length()-1);
                    }
                    break;
                }
                break;
            case SDL_TEXTINPUT:
                //Add new text onto the end of our text
                textTerminal[currentLine].append(e.text.text);
                break;
            case SDL_TEXTEDITING:
                //Update the composition text.
                //Update the cursor position.
                //Update the selection length (if any).
                //text.append(event.edit.text);
                break;
            }
        }
        //Rendering
        SDL_RenderClear(rendererTerminal);
        SDL_RenderClear(rendererMain);
        
        //Draw the background grey
        boxRGBA(rendererMain, 0, 0, current.w, current.h, 20, 20, 20, 0);
        
        //render terminal text
        for(int i=0; i<(int)textTerminal.size(); i++) {
            texture = renderText(textTerminal[i].c_str(), "monaco.ttf", colorWhite, 28, rendererTerminal);
            renderTexture(texture, rendererTerminal, 0, i*35);
        }
        
        if(showSplash) {
            textMain = "Familienduell";
            texture = renderText(textMain, "lazy.ttf", colorGreen, 3*current.h/NUMBER_OF_LINES, rendererMain);
            renderTexture(texture, rendererMain, 1*current.w/NUMBER_OF_COLUMNS, 3*current.h/NUMBER_OF_LINES);
            
            textMain = "<type start>";
            texture = renderText(textMain, "lazy.ttf", colorGreen, 1*current.h/NUMBER_OF_LINES, rendererMain);
            SDL_SetTextureAlphaMod(texture, (int)worldtime.getTicks()/2);
            renderTexture(texture, rendererMain, 6*current.w/NUMBER_OF_COLUMNS, 7*current.h/NUMBER_OF_LINES);
        }
            
        //reveal the Answers
        if(showAnswers) {
            textMain = interface.getQuestion(currentQuestion);
            texture = renderText(textMain, "lazy.ttf", colorGreen, current.h/NUMBER_OF_LINES, rendererMain);
            renderTexture(texture, rendererMain, 1*current.w/NUMBER_OF_COLUMNS, 0*current.h/NUMBER_OF_LINES);
            
            for(int i=1; i<=4; i++) {
                textMain = interface.getAnswer(currentQuestion,i);
                texture = renderText(textMain, "lazy.ttf", colorGreen, current.h/NUMBER_OF_LINES, rendererMain);
                renderTexture(texture, rendererMain, 1*current.w/NUMBER_OF_COLUMNS, (i+1)*current.h/NUMBER_OF_LINES);
            }
        }
        
        //apply to the screen
        SDL_RenderPresent(rendererTerminal);
        SDL_RenderPresent(rendererMain);
        
        //Timer related stuff
        frame++;
        if( (fps.getTicks() < 1000/FRAMES_PER_SECOND) ) {
            SDL_Delay( (1000/FRAMES_PER_SECOND) - fps.getTicks() );
        }
        SDL_StopTextInput();
    }

    //destroy the items
	cleanup(rendererMain, windowMain);
    cleanup(rendererTerminal, windowTerminal);
	IMG_Quit();
	SDL_Quit();
    TTF_Quit();
	return 0;
}