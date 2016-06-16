#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_gfx/SDL2_gfxPrimitives.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <math.h>
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
const int FULLSCREEN_WIDTH = 1440;
const int FULLSCREEN_HEIGHT = 900;
const int NUMBER_OF_LINES = 13;
const int NUMBER_OF_COLUMNS = 20;
const int FRAMES_PER_SECOND = 30;

 //TODO, verallgemeinern!
const int NUMBER_OF_QUESTIONS = 2;
const int NUMBER_OF_ANSWERS = 5;


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
    SDL_DestroyTexture(texture);
}

//Render text
void renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int alpha, int fontSize, SDL_Renderer *renderer, int x, int y) {

	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if(font == nullptr) {
        std::cout<<"Error 1 in rencerText.";
    }
    
	SDL_Surface *surf = TTF_RenderUTF8_Blended(font, message.c_str(), color);
    if(surf == nullptr) {
        std::cout<<"Error 2 in rencerText.";
    }
    
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if(texture == nullptr) {
        std::cout<<"Error 3 in rencerText.";
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
        std::cout<<"Error 1 in rencerText.";
    }
    
	SDL_Surface *surf = TTF_RenderUTF8_Blended(font, message.c_str(), color);
    if(surf == nullptr) {
        std::cout<<"Error 2 in rencerText.";
    }
    
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if(texture == nullptr) {
        std::cout<<"Error 3 in rencerText.";
    }
    
    renderTexture(texture, renderer, x, y);
    
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
}

int main(int argc, char* args[]) {
    //load the questions and the answers
    Interface interface("Fragen.txt");
    
    //the textures we want to render
    SDL_Texture *texture = NULL;
    SDL_Texture *image = NULL;
    
    //The text that's going to be used
    std::vector<std::string> textTerminal;
    std::vector<std::string> history;
    textTerminal.push_back("fd> ");
    int currentLine = 0;
    int selectCommand = 0;
    string textMain = " ";
    SDL_Color colorGreen = { 173, 200, 0, 255};
    SDL_Color colorWhite = { 255, 255, 255, 255};
    SDL_Color colorBlack = { 20, 20, 20, 255};
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
    bool showMain = false;
    bool displayText = false;
    std::string dText = std::string();
    int currentQuestion = 0;
    bool showAnswer[NUMBER_OF_ANSWERS+1];
    for(int i=0; i<=NUMBER_OF_ANSWERS; i++) showAnswer[i] = false;
    int pointsA = 0, pointsB = 0, points = 0;
    //number of wrong answers
    int nWrongA = 0, nWrongB = 0;
    //the group which won the first question and has now the current turn
    std::string currentGroup = "A";
    
    //Setup our window and renderer, this is the public screen
    SDL_Window *windowTerminal = SDL_CreateWindow("Familienduell", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, TERMINAL_WIDTH, TERMINAL_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *windowMain = SDL_CreateWindow("Familienduell", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	if (windowMain == NULL || windowTerminal == NULL){
		std::cout<<"Error in CreateWindow"<<std::endl;
		SDL_Quit();
		return 1;
	}
    SDL_Renderer *rendererTerminal = SDL_CreateRenderer(windowTerminal, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Renderer *rendererMain = SDL_CreateRenderer(windowMain, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (rendererMain == NULL || rendererTerminal == NULL){
		std::cout<<"Error in CreateRenderer"<<std::endl;
		cleanup(windowMain);
        cleanup(windowTerminal);
		SDL_Quit();
		return 1;
	}
    SDL_RaiseWindow(windowTerminal);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(rendererMain, SCREEN_WIDTH, SCREEN_HEIGHT);
    
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
        int width = SCREEN_WIDTH, height = SCREEN_HEIGHT;
        SDL_GetWindowSize(windowMain,&width,&height);
        //std::cout<<width<<", "<<height<<std::endl;
        
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
                    //if end of terminal reached, erase first element
                    if(currentLine>14) {
                        textTerminal.erase(textTerminal.begin());
                        currentLine = 14;
                    }
                    textTerminal.push_back("fd> ");
                    //handle terminal commands
                    if(currentLine>=0) {
                        std::string temp = textTerminal[currentLine];
                        std::vector<std::string> commandVec;
                        if(temp.length()>4) {
                            temp = temp.substr(4,temp.length());
                            history.push_back(temp);
                            //split command into multiple strings
                            commandVec = strToVec(temp);
                            for(int i=0; i<(int)commandVec.size(); i++) {
                                std::cout<<commandVec[i]<<std::endl;
                            }
                            selectCommand = history.size();
                            //no command selection
                            if(commandVec[0].compare("fullscreen") == 0) {
                                toggleFullscreen(windowMain, rendererMain);
                                SDL_RaiseWindow(windowTerminal);
                            }
                            if(commandVec[0].compare("exit") == 0) {
                                quit = true;
                            }
                            if(commandVec[0].compare("next") == 0) {
                                currentQuestion++;
                                if(currentQuestion<0) currentQuestion = 0;
                                nWrongA = 0; nWrongB = 0;
                                points = 0;
                                for(int i=0; i<=NUMBER_OF_ANSWERS; i++) showAnswer[i] = false;
                            }
                            if(commandVec[0].compare("previous") == 0) {
                                currentQuestion--;
                                if(currentQuestion>=NUMBER_OF_QUESTIONS) currentQuestion = NUMBER_OF_QUESTIONS-1;
                            }
                            if(commandVec[0].compare("start") == 0) {
                                showSplash = false;
                                showMain = true;
                                displayText = false;
                            }
                            if(commandVec[0].compare("set") == 0) {
                                if(commandVec[1].compare("A") == 0 or commandVec[1].compare("B") == 0) {
                                    currentGroup = commandVec[1];
                                }
                            }
                            if(commandVec[0].compare("reveal") == 0) {
                                if(is_number(commandVec[1])) {
                                    std::string::size_type sz;
                                    int n = std::stoi(commandVec[1],&sz);
                                    if(n>0 and n<=NUMBER_OF_ANSWERS) {
                                        showAnswer[n] = true;
                                        points += interface.getAnswerPoints(currentQuestion,n);
                                    }
                                    //if the group has (correctly) revealed all the answers, set showAnswer[0] to true
                                    showAnswer[0] = true;
                                    for(int i=1; i<=NUMBER_OF_ANSWERS; i++) {
                                        if(!showAnswer[i]) showAnswer[0] = false;
                                    }
                                    //take the points
                                    if(showAnswer[0] && currentGroup.compare("A") == 0) {
                                        pointsA += points;
                                        points = 0;
                                    } else if(showAnswer[0] && currentGroup.compare("B") == 0) {
                                        pointsB += points;
                                        points = 0;
                                    }
                                }
                            }
                            if(commandVec[0].compare("restart") == 0) {
                                showSplash = true;
                                showMain = false;
                                displayText = false;
                                currentQuestion = 0;
                                pointsA = 0; pointsB = 0;
                                points = 0;
                                for(int i=0; i<=NUMBER_OF_ANSWERS; i++) showAnswer[i] = false;
                            }
                            if(commandVec[0].compare("display") == 0) {
                                //TODO: save previous state and get back to it with command "back"
                                dText = temp.substr(8,temp.length());
                                displayText = true;
                                showSplash = false;
                                showMain = false;
                            }
                            if(showMain && commandVec[0].compare("wrong") == 0) {
                                if(currentGroup.compare("A") == 0 && commandVec[1].compare("A") == 0) {
                                    nWrongA += 1;
                                    if(nWrongA>=3) {
                                        nWrongA = 3;
                                        currentGroup = "B";
                                    }
                                } else if(currentGroup.compare("B") == 0 && commandVec[1].compare("B") == 0) {
                                    nWrongB += 1;
                                    if(nWrongB>=3) {
                                        nWrongB = 3;
                                        currentGroup = "A";
                                    }
                                }
                            }
                            if(showMain && commandVec[0].compare("add") == 0) {
                                if(commandVec[1].compare("A") == 0) {
                                    if(is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsA += std::stoi(commandVec[2],&sz);
                                    }
                                } else if(commandVec[1].compare("B") == 0) {
                                    if(is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsB += std::stoi(commandVec[2],&sz);
                                    }
                                } else {
                                    if(is_number(commandVec[1])) {
                                        std::string::size_type sz;
                                        points += std::stoi(commandVec[1],&sz);
                                    }
                                }
                            }
                            if(showMain && commandVec[0].compare("sub") == 0) {
                                if(commandVec[1].compare("A") == 0) {
                                    if(is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsA -= std::stoi(commandVec[2],&sz);
                                    }
                                } else if(commandVec[1].compare("B") == 0) {
                                    if(is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsB -= std::stoi(commandVec[2],&sz);
                                    }
                                } else {
                                    if(is_number(commandVec[1])) {
                                        std::string::size_type sz;
                                        points -= std::stoi(commandVec[1],&sz);
                                    }
                                }
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
            case SDL_WINDOWEVENT:
                switch(e.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    //std::cout<<e.window.windowID<<std::endl;
                    //std::cout<<e.window.data1<<std::endl;
                    //std::cout<<e.window.data2<<std::endl;
                    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
                    SDL_RenderSetLogicalSize(rendererMain, e.window.data1, e.window.data2);
                }
            }
        }
        //Rendering
        SDL_RenderClear(rendererTerminal);
        SDL_RenderClear(rendererMain);
        
        
        //Draw the background grey
        boxRGBA(rendererMain, 0, 0, width, height, 20, 20, 20, 255);
        
        //show grid (debugging)
        /*
        for(int i=0; i<NUMBER_OF_COLUMNS; i++) {
            lineRGBA(rendererMain, i*width/NUMBER_OF_COLUMNS, 0, i*width/NUMBER_OF_COLUMNS, height, 255, 0, 0, 200);
        }
        for(int j=0; j<NUMBER_OF_LINES; j++) {
            lineRGBA(rendererMain, 0, j*height/NUMBER_OF_LINES, width, j*height/NUMBER_OF_LINES, 255, 0, 0, 200);
        }
        */
        
        //render terminal text
        for(int i=0; i<(int)textTerminal.size(); i++) {
            renderText(textTerminal[i].c_str(), "monaco.ttf", colorWhite, 28, rendererTerminal, 0, i*35);
        }
        
        //start screen
        if(showSplash) {
            textMain = "Familienduell";
            renderText(textMain, "lazy.ttf", colorGreen, 2.7*height/NUMBER_OF_LINES, rendererMain, 
                        1*width/NUMBER_OF_COLUMNS, 3*height/NUMBER_OF_LINES);
            
            textMain = "<type start>";
            renderText(textMain, "lazy.ttf", colorGreen, (int)worldtime.getTicks()/2, 
                        1*height/NUMBER_OF_LINES, rendererMain, 6*width/NUMBER_OF_COLUMNS, 7*height/NUMBER_OF_LINES);
        }
         
        //main screen
        if(showMain) {
            textMain = interface.getQuestion(currentQuestion);
            renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain,
                        1*width/NUMBER_OF_COLUMNS, 0*height/NUMBER_OF_LINES);
            
            //draw line
            lineRGBA(rendererMain, 0, 1.5*height/NUMBER_OF_LINES, width, 1.5*height/NUMBER_OF_LINES, 173, 200, 0, 255);
            
            //draw bottom box
            boxRGBA(rendererMain, 0, 11*height/NUMBER_OF_LINES, width, height, 173, 200, 0, 200);
            
            //draw a box around the points of current group
            if(currentGroup.compare("A") == 0) {
                if(pointsA>=1000) {
                    boxRGBA(rendererMain, 0.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            5.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 0.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                            5.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if(pointsA>=100) {
                    boxRGBA(rendererMain, 0.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            4.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 0.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                            4.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if(pointsA>=10) {
                    boxRGBA(rendererMain, 0.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            3.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 0.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                            3.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else {
                    boxRGBA(rendererMain, 0.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            2.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 0.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                            2.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                }
            } else if(currentGroup.compare("B") == 0) {
                if(pointsB>=1000) {
                    boxRGBA(rendererMain, 14.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            19.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 14.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                        19.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if(pointsB>=100) {
                    boxRGBA(rendererMain, 15.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            19.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 15.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                        19.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if(pointsB>=10) {
                    boxRGBA(rendererMain, 16.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            19.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 16.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                        19.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else {
                    boxRGBA(rendererMain, 17.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            19.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 17.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                        19.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                }
            }
            
            //show answers and the underlaying layout
            for(int i=1; i<=NUMBER_OF_ANSWERS; i++) {
                textMain = std::to_string(i) + "." ;
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain,
                            1*width/NUMBER_OF_COLUMNS, (1*i+1)*height/NUMBER_OF_LINES);
                if(!showAnswer[i]) {
                    textMain = "- -" ;
                    renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                                18*width/NUMBER_OF_COLUMNS, (1*i+1)*height/NUMBER_OF_LINES);
                }
            }
            
            for(int i=1; i<=NUMBER_OF_ANSWERS; i++) {
                if(showAnswer[i]) {
                    textMain = interface.getAnswer(currentQuestion,i);
                    renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                                2.5*width/NUMBER_OF_COLUMNS, (1*i+1)*height/NUMBER_OF_LINES);
                    textMain = std::to_string(interface.getAnswerPoints(currentQuestion,i));
                    renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain,
                                18*width/NUMBER_OF_COLUMNS, (1*i+1)*height/NUMBER_OF_LINES);
                }
            }
            
            //display points
            //Group A
            textMain = std::to_string(pointsA);
            renderText(textMain, "lazy.ttf", colorBlack, 1.5*height/NUMBER_OF_LINES, rendererMain,
                        1*width/NUMBER_OF_COLUMNS, 11*height/NUMBER_OF_LINES);
            
            //Group B
            textMain = std::to_string(pointsB);
            if(pointsB>=1000) {
                renderText(textMain, "lazy.ttf", colorBlack, 1.5*height/NUMBER_OF_LINES, rendererMain,
                            15*width/NUMBER_OF_COLUMNS, 11*height/NUMBER_OF_LINES);
            } else if(pointsB>=100) {
                renderText(textMain, "lazy.ttf", colorBlack, 1.5*height/NUMBER_OF_LINES, rendererMain,
                            16*width/NUMBER_OF_COLUMNS, 11*height/NUMBER_OF_LINES);
            } else if(pointsB>=10) {
                renderText(textMain, "lazy.ttf", colorBlack, 1.5*height/NUMBER_OF_LINES, rendererMain,
                            17*width/NUMBER_OF_COLUMNS, 11*height/NUMBER_OF_LINES);
            } else {
                renderText(textMain, "lazy.ttf", colorBlack, 1.5*height/NUMBER_OF_LINES, rendererMain,
                            18*width/NUMBER_OF_COLUMNS, 11*height/NUMBER_OF_LINES);
            }
            
            //point of each turn (the points can get stolen, if the group has three wrong answers)
            textMain = "Summe: ";
            renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                        12*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            textMain = std::to_string(points);
            if(points>=100) {
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                            17*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            } else if(points>=10) {
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                            17.5*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            } else {
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                            18*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            }
            
            //display x for wrong answers
            if(nWrongA > 0 or nWrongB > 0) {
                image = IMG_LoadTexture(rendererMain, "false.png");
                for (int i=1; i<=nWrongA; i++) {
                    renderTexture(image, rendererMain, (i)*width/NUMBER_OF_COLUMNS, 10*height/NUMBER_OF_LINES, width/NUMBER_OF_COLUMNS, height/NUMBER_OF_LINES);
                }
                for (int i=1; i<=nWrongB; i++) {
                    renderTexture(image, rendererMain, (15+i)*width/NUMBER_OF_COLUMNS, 10*height/NUMBER_OF_LINES, width/NUMBER_OF_COLUMNS, height/NUMBER_OF_LINES);
                }
            }
        }
        
        //display text on screen
        if(displayText) {
            textMain = dText;
            renderText(textMain, "lazy.ttf", colorGreen, 3*height/NUMBER_OF_LINES, rendererMain, 
                        1*width/NUMBER_OF_COLUMNS, 3*height/NUMBER_OF_LINES);
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
	cleanup(rendererMain, windowMain, rendererTerminal, windowTerminal, texture, image);
	IMG_Quit();
	SDL_Quit();
    TTF_Quit();
	return 0;
}