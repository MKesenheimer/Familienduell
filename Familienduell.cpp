#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_gfx/SDL2_gfxPrimitives.h>
#include <SDL2_ttf/SDL_ttf.h>
#include "SDL2_mixer/SDL_mixer.h"
#include <math.h>
#include "SDL_Defs.h"
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

 //TODO, verallgemeinern!
const size_t NUMBER_OF_QUESTIONS = 14;
const size_t NUMBER_OF_ANSWERS = 4;

using namespace::std;

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
    size_t currentLine = 0;
    size_t selectCommand = 0;
    string textMain = " ";
    SDL_Color colorGreen = {173, 200, 0, 255};
    SDL_Color colorWhite = {255, 255, 255, 255};
    SDL_Color colorBlack = {20, 20, 20, 255};
    //Initialize SDL_ttf
    if (TTF_Init() != 0) {
        std::cout << "Error in TTF_Init" << std::endl;
        return 1;
    }

	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "Error in SDL_Init" << std::endl;
		return 1;
	}
    
    //initialize audio
    int flags = MIX_INIT_MP3;
    if (flags != Mix_Init(MIX_INIT_MP3)) {
        std::cout << "Error in Mix_init" << std::endl;
        return 1;
    }
    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
    Mix_Music *mix_intro = Mix_LoadMUS("intro.mp3");
    Mix_Music *mix_wrong = Mix_LoadMUS("wrong.mp3");
    Mix_Music *mix_reveal = Mix_LoadMUS("reveal.mp3");
    
    //Setup our window and renderer, this is the public screen
    SDL_Window *windowTerminal = SDL_CreateWindow("Familienduell", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, TERMINAL_WIDTH, TERMINAL_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *windowMain = SDL_CreateWindow("Familienduell", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	if (windowMain == NULL || windowTerminal == NULL){
		std::cout << "Error in CreateWindow" << std::endl;
		SDL_Quit();
		return 1;
	}
    SDL_Renderer *rendererTerminal = SDL_CreateRenderer(windowTerminal, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Renderer *rendererMain = SDL_CreateRenderer(windowMain, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (rendererMain == NULL || rendererTerminal == NULL){
		std::cout << "Error in CreateRenderer" << std::endl;
		cleanup(windowMain);
        cleanup(windowTerminal);
		SDL_Quit();
		return 1;
	}
    SDL_RaiseWindow(windowTerminal);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(rendererMain, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    //Gamelogic
    bool showSplash = true;
    bool showMain = false;
    bool showEnd = false;
    bool showLogo = false;
    bool showPigs = false;
    bool displayText = false;
    std::string dText = std::string();
    size_t currentQuestion = 0;
    bool showAnswer[NUMBER_OF_ANSWERS+1];
    for (size_t i = 0; i <= NUMBER_OF_ANSWERS; i++) showAnswer[i] = false;
    int pointsA = 0, pointsB = 0, points = 0;
    //number of wrong answers
    int nWrongA = 0, nWrongB = 0;
    //the group which won the first question and has now the current turn
    std::string currentGroup = "A";
    //Timer zum festlegen der FPS
	Timer fps;
    Timer worldtime;
    worldtime.start();
    //number of frame
	int frame = 0;
    
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
        SDL_GetWindowSize(windowMain, &width, &height);
        //std::cout << width << ", " << height << std::endl;
        
		while(SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                SDL_RaiseWindow(windowTerminal);
                switch(e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    toggleFullscreen(windowMain, rendererMain);
                    SDL_RaiseWindow(windowTerminal);
                    break;
                case SDLK_RETURN:
                    //if end of terminal reached, erase first element
                    if (currentLine > 14) {
                        textTerminal.erase(textTerminal.begin());
                        currentLine = 14;
                    }
                    textTerminal.push_back("fd> ");
                    //handle terminal commands
                    if (currentLine >= 0) {
                        std::string temp(textTerminal[currentLine]);
                        std::vector<std::string> commandVec;
                        if (temp.length()>4) {
                            temp = temp.substr(4, temp.length());
                            history.push_back(temp);
                            //split command into multiple strings
                            commandVec = strToVec(temp);
                            for (size_t i = 0; i < commandVec.size(); i++) {
                                std::cout << commandVec[i] << std::endl;
                            }
                            selectCommand = history.size();
                            if (commandVec[0].compare("help") == 0) {
                                textTerminal.push_back(" fullscreen");
                                textTerminal.push_back(" exit");
                                textTerminal.push_back(" next");
                                textTerminal.push_back(" previous");
                                textTerminal.push_back(" start");
                                textTerminal.push_back(" end | quit");
                                textTerminal.push_back(" set <group A | B>");
                                textTerminal.push_back(" reveal <# of answer>");
                                textTerminal.push_back(" hide   <# of answer>");
                                textTerminal.push_back(" display <text>");
                                textTerminal.push_back("fd> ");
                                currentLine += 11;
                            }
                            //no command selection
                            if (commandVec[0].compare("fullscreen") == 0) {
                                toggleFullscreen(windowMain, rendererMain);
                                SDL_RaiseWindow(windowTerminal);
                            }
                            if (commandVec[0].compare("exit") == 0) {
                                quit = true;
                            }
                            if (commandVec[0].compare("next") == 0 and (showLogo or showMain or showPigs)) {
                                showPigs = false;
                                showMain = true;
                                if (currentQuestion >= NUMBER_OF_QUESTIONS) currentQuestion = NUMBER_OF_QUESTIONS - 1;
                                if (!showLogo) {
                                    if (currentQuestion < (NUMBER_OF_QUESTIONS - 1)) {
                                        currentQuestion++;
                                        //take the points 
                                        if (currentGroup.compare("A") == 0 and nWrongA < 3) {
                                            pointsA += points;
                                        } else if (currentGroup.compare("B") == 0 and nWrongB < 3) {
                                            pointsB += points;
                                        }
                                        nWrongA = 0; nWrongB = 0;
                                        points = 0;
                                        for (size_t i = 0; i <= NUMBER_OF_ANSWERS; i++) showAnswer[i] = false;
                                    //end of game
                                    } else {
                                        Mix_PlayMusic(mix_intro, 1);
                                        currentQuestion = NUMBER_OF_QUESTIONS;
                                        showEnd = true;
                                        showMain = false;
                                        showSplash = false;
                                        showLogo = false;
                                        showPigs = false;
                                        displayText = false;
                                    }
                                } else {
                                    //if the logo is being displayed
                                    showEnd = false;
                                    showMain = true;
                                    showSplash = false;
                                    showLogo = false;
                                    showPigs = false;
                                    displayText = false;
                                }
                            }
                            //the previous command works only in main screen
                            if (commandVec[0].compare("previous") == 0) {
                                if (currentQuestion <= 0) currentQuestion = 0;
                                if (currentQuestion>0) {
                                    currentQuestion--;
                                    for (size_t i = 0; i <= NUMBER_OF_ANSWERS; i++) showAnswer[i] = false;
                                    nWrongA = 0; nWrongB = 0;
                                    points = 0;
                                    showSplash = false;
                                    showMain = true;
                                    showLogo = false;
                                    showPigs = false;
                                    displayText = false;
                                    showEnd = false;
                                }
                            }
                            if (commandVec[0].compare("start") == 0) {
                                Mix_PlayMusic(mix_intro, 1);
                                showLogo =true;
                                showSplash = false;
                                showMain = false;
                                displayText = false;
                                showEnd = false;
                                showPigs = false;
                            }
                            if (commandVec[0].compare("end") == 0 || commandVec[0].compare("quit") == 0) {
                                Mix_PlayMusic(mix_intro, 1);
                                currentQuestion = NUMBER_OF_QUESTIONS;
                                showEnd = true;
                                showMain = false;
                                showSplash = false;
                                showLogo = false;
                                showPigs = false;
                                displayText = false;
                            }
                            if (commandVec[0].compare("set") == 0) {
                                if (commandVec[1].compare("A") == 0 or commandVec[1].compare("B") == 0) {
                                    currentGroup = commandVec[1];
                                }
                            }
                            if (commandVec[0].compare("reveal") == 0) {
                                if (is_number(commandVec[1])) {
                                    std::string::size_type sz;
                                    size_t n = static_cast<size_t>(std::stoi(commandVec[1], &sz));
                                    if (n > 0 and n <= NUMBER_OF_ANSWERS and (nWrongA < 3 or nWrongB < 3) and !showAnswer[n]) {
                                        Mix_PlayMusic(mix_reveal, 1);
                                        showAnswer[n] = true;
                                        points += interface.getAnswerPoints(currentQuestion,n);
                                    }
                                    //if the group has (correctly) revealed all the answers, set showAnswer[0] to true
                                    showAnswer[0] = true;
                                    for (size_t i = 1; i <= NUMBER_OF_ANSWERS; i++) {
                                        if (!showAnswer[i]) showAnswer[0] = false;
                                    }
                                    //take the points
                                    if (showAnswer[0] and currentGroup.compare("A") == 0 and nWrongA < 3) {
                                        pointsA += points;
                                        points = 0;
                                    } else if (showAnswer[0] and currentGroup.compare("B") == 0 and nWrongB < 3) {
                                        pointsB += points;
                                        points = 0;
                                    }
                                    //if group A has three wrong answers, it's group B's turn
                                    //if group B (correctly) reveals one answer, they steal the points of group A
                                    if (nWrongA >= 3 and nWrongB < 3) {
                                        pointsB += points;
                                        points = 0;
                                    } else if (nWrongB >= 3 and nWrongA < 3) {
                                        pointsA += points;
                                        points = 0;
                                    }
                                //this is for testing purpose only (to check the text, not the functionality)
                                } else if (commandVec[1].compare("all") == 0) {
                                    for (size_t i = 1; i <= NUMBER_OF_ANSWERS; i++) {
                                        showAnswer[i] = true;
                                    }
                                }
                            }
                            if (commandVec[0].compare("hide") == 0) {
                                if (is_number(commandVec[1])) {
                                    std::string::size_type sz;
                                    size_t n = static_cast<size_t>(std::stoi(commandVec[1], &sz));
                                    if (n > 0 and n <= NUMBER_OF_ANSWERS and (nWrongA < 3 or nWrongB < 3)) {
                                        showAnswer[n] = false;
                                        points -= interface.getAnswerPoints(currentQuestion,n);
                                    }
                                }
                            }
                            if (commandVec[0].compare("restart") == 0) {
                                showSplash = true;
                                showLogo = false;
                                showPigs = false;
                                showMain = false;
                                displayText = false;
                                currentQuestion = 0;
                                pointsA = 0; pointsB = 0;
                                points = 0;
                                for (size_t i = 0; i <= NUMBER_OF_ANSWERS; i++) showAnswer[i] = false;
                                nWrongA = 0; nWrongB = 0;
                                showEnd = false;
                            }
                            //only possible in main
                            if (showMain and commandVec[0].compare("display") == 0) {
                                if (temp.length() > 8) {
                                    dText = temp.substr(8, temp.length());
                                    displayText = true;
                                    showSplash = false;
                                    showMain = false;
                                    showEnd = false;
                                    showLogo = false;
                                    showPigs = false;
                                }
                            }
                            //only possible in main
                            if (showMain and commandVec[0].compare("pigs") == 0) {
                                displayText = false;
                                showSplash = false;
                                showMain = false;
                                showEnd = false;
                                showLogo = false;
                                showPigs = true;
                            }
                            if ((displayText or showPigs) and commandVec[0].compare("back") == 0) {
                                dText = "";
                                displayText = false;
                                showSplash = false;
                                showMain = true;
                                showEnd = false;
                                showLogo = false;
                                showPigs = false;
                            }
                            if (showMain and (commandVec[0].compare("wrong") == 0 || commandVec[0].compare("false") == 0)) {
                                if (currentGroup.compare("A") == 0 and commandVec[1].compare("A") == 0) {
                                    nWrongA += 1;
                                    if (nWrongA >= 3) {
                                        nWrongA = 3;
                                        currentGroup = "B";
                                    }
                                    Mix_PlayMusic(mix_wrong, 1);
                                } else if (currentGroup.compare("B") == 0 and commandVec[1].compare("B") == 0) {
                                    nWrongB += 1;
                                    if (nWrongB >= 3) {
                                        nWrongB = 3;
                                        currentGroup = "A";
                                    }
                                    Mix_PlayMusic(mix_wrong, 1);
                                }
                                if (nWrongB >= 3 and nWrongA >= 3) {
                                    points = 0;
                                }
                            }
                            if (showMain and commandVec[0].compare("add") == 0) {
                                if (commandVec[1].compare("A") == 0) {
                                    if (is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsA += std::stoi(commandVec[2], &sz);
                                    }
                                } else if (commandVec[1].compare("B") == 0) {
                                    if (is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsB += std::stoi(commandVec[2], &sz);
                                    }
                                } else {
                                    if (is_number(commandVec[1])) {
                                        std::string::size_type sz;
                                        points += std::stoi(commandVec[1], &sz);
                                    }
                                }
                            }
                            if (showMain and commandVec[0].compare("sub") == 0) {
                                if (commandVec[1].compare("A") == 0) {
                                    if (is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsA -= std::stoi(commandVec[2], &sz);
                                    }
                                } else if (commandVec[1].compare("B") == 0) {
                                    if (is_number(commandVec[2])) {
                                        std::string::size_type sz;
                                        pointsB -= std::stoi(commandVec[2], &sz);
                                    }
                                } else {
                                    if (is_number(commandVec[1])) {
                                        std::string::size_type sz;
                                        points -= std::stoi(commandVec[1], &sz);
                                    }
                                }
                            }
                        }
                    }
                    currentLine++;
                    break;
                case SDLK_UP:
                    selectCommand--;
                    if (history.size() > 0 and selectCommand >= 0) {
                        textTerminal[currentLine] = "fd> " + history[selectCommand];
                    } else {
                        selectCommand = 0;
                    }
                    break;
                case SDLK_DOWN:
                    selectCommand++;
                    if (selectCommand <= history.size()) {
                        textTerminal[currentLine] = "fd> " + history[selectCommand];
                    } else {
                        selectCommand = history.size();
                    }
                    break;
                case SDLK_BACKSPACE:
                    std::string temp = textTerminal[currentLine];
                    if (temp.length() > 4) {
                        textTerminal[currentLine] = temp.substr(0, temp.length() - 1);
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
                    //std::cout << e.window.windowID << std::endl;
                    //std::cout << e.window.data1 << std::endl;
                    //std::cout << e.window.data2 << std::endl;
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
        
        //render terminal text
        for (size_t i = 0; i < textTerminal.size(); i++) {
            renderText(textTerminal[i].c_str(), "monaco.ttf", colorWhite, 28, rendererTerminal, 0, i*35);
        }
        
        //start screen
        if (showSplash) {
            textMain = "Familienduell";
            renderText(textMain, "lazy.ttf", colorGreen, 2.7*height/NUMBER_OF_LINES, rendererMain, 
                        1*width/NUMBER_OF_COLUMNS, 3*height/NUMBER_OF_LINES);
            
            textMain = "<type start>";
            renderText(textMain, "lazy.ttf", colorGreen, static_cast<int>(worldtime.getTicks()/2.0),
                        1*height/NUMBER_OF_LINES, rendererMain, 6*width/NUMBER_OF_COLUMNS, 7*height/NUMBER_OF_LINES);
        }

        if (showLogo) {
            //play tingle and show logo
            image = IMG_LoadTexture(rendererMain, "logo.png");
            renderTexture(image, rendererMain, 0, 0, width, height);
        }
        
        if (showPigs) {
            //display pigs
            image = IMG_LoadTexture(rendererMain, "pigs.png");
            renderTexture(image, rendererMain, 0, 0, width, height);
        }
        
        //main screen
        if (showMain) {
            textMain = interface.getQuestion(currentQuestion);
            renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain,
                        1*width/NUMBER_OF_COLUMNS, 0*height/NUMBER_OF_LINES);
            
            //draw line
            lineRGBA(rendererMain, 0, 1.5*height/NUMBER_OF_LINES, width, 1.5*height/NUMBER_OF_LINES, 173, 200, 0, 255);
            
            //draw bottom box
            boxRGBA(rendererMain, 0, 11*height/NUMBER_OF_LINES, width, height, 173, 200, 0, 200);
            
            //draw a box around the points of current group
            if (currentGroup.compare("A") == 0) {
                if (pointsA >= 1000) {
                    boxRGBA(rendererMain, 0.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            5.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 0.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                            5.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if (pointsA >= 100) {
                    boxRGBA(rendererMain, 0.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            4.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 0.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                            4.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if (pointsA >= 10) {
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
            } else if (currentGroup.compare("B") == 0) {
                if (pointsB >= 1000) {
                    boxRGBA(rendererMain, 14.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            19.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 14.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                        19.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if (pointsB >= 100) {
                    boxRGBA(rendererMain, 15.2*width/NUMBER_OF_COLUMNS, 11.2*height/NUMBER_OF_LINES, 
                            19.8*width/NUMBER_OF_COLUMNS, 12.8*height/NUMBER_OF_LINES, 20, 20, 20, 255);
                    boxRGBA(rendererMain, 15.3*width/NUMBER_OF_COLUMNS, 11.3*height/NUMBER_OF_LINES, 
                        19.7*width/NUMBER_OF_COLUMNS, 12.7*height/NUMBER_OF_LINES, 173, 200, 0, 200);
                } else if (pointsB >= 10) {
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
            for (size_t i = 1; i <= NUMBER_OF_ANSWERS; i++) {
                textMain = std::to_string(i) + "." ;
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain,
                            1*width/NUMBER_OF_COLUMNS, (1*i+1)*height/NUMBER_OF_LINES);
                if (!showAnswer[i]) {
                    textMain = "- -" ;
                    renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                                18*width/NUMBER_OF_COLUMNS, (1*i+1)*height/NUMBER_OF_LINES);
                }
            }
            
            for (size_t i = 1; i <= NUMBER_OF_ANSWERS; i++) {
                if (showAnswer[i]) {
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
            if (pointsB >= 1000) {
                renderText(textMain, "lazy.ttf", colorBlack, 1.5*height/NUMBER_OF_LINES, rendererMain,
                            15*width/NUMBER_OF_COLUMNS, 11*height/NUMBER_OF_LINES);
            } else if (pointsB >= 100) {
                renderText(textMain, "lazy.ttf", colorBlack, 1.5*height/NUMBER_OF_LINES, rendererMain,
                            16*width/NUMBER_OF_COLUMNS, 11*height/NUMBER_OF_LINES);
            } else if (pointsB >= 10) {
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
            if (points >= 100) {
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                            17*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            } else if (points >= 10) {
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                            17.5*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            } else {
                renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                            18*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            }
            
            //display x for wrong answers
            if (nWrongA > 0 or nWrongB > 0) {
                image = IMG_LoadTexture(rendererMain, "false.png");
                for (int i = 1; i <= nWrongA; i++) {
                    renderTexture(image, rendererMain, (i)*width/NUMBER_OF_COLUMNS, 10*height/NUMBER_OF_LINES, width/NUMBER_OF_COLUMNS, height/NUMBER_OF_LINES);
                }
                for (int i = 1; i <= nWrongB; i++) {
                    renderTexture(image, rendererMain, (15+i)*width/NUMBER_OF_COLUMNS, 10*height/NUMBER_OF_LINES, width/NUMBER_OF_COLUMNS, height/NUMBER_OF_LINES);
                }
            }
        }
        
        //display text on screen
        if (displayText) {
            textMain = dText;
            renderText(textMain, "lazy.ttf", colorGreen, 3*height/NUMBER_OF_LINES, rendererMain, 
                        1*width/NUMBER_OF_COLUMNS, 3*height/NUMBER_OF_LINES);
        }
        
        if (showEnd) {
        
            textMain = "Vielen Dank fürs Mitmachen!";
            renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                        2*width/NUMBER_OF_COLUMNS, 2*height/NUMBER_OF_LINES);
            /*
            textMain = "Gottes Segen und";
            renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                        2*width/NUMBER_OF_COLUMNS, 4*height/NUMBER_OF_LINES);
            textMain = "ein gutes neues Jahr!";
            renderText(textMain, "lazy.ttf", colorGreen, 1*height/NUMBER_OF_LINES, rendererMain, 
                        2*width/NUMBER_OF_COLUMNS, 6*height/NUMBER_OF_LINES);
            */
            
            /*
            textMain = "Alles Gute!";
            renderText(textMain, "lazy.ttf", colorGreen, 2*height/NUMBER_OF_LINES, rendererMain, 
                        2*width/NUMBER_OF_COLUMNS, 2*height/NUMBER_OF_LINES);
            textMain = "wünschen euch:";
            renderText(textMain, "lazy.ttf", colorGreen, 0.8*height/NUMBER_OF_LINES, rendererMain, 
                        2.5*width/NUMBER_OF_COLUMNS, 5*height/NUMBER_OF_LINES);
            textMain = "Jenny, Matze, Kadda, Johannes,";
            renderText(textMain, "lazy.ttf", colorGreen, 0.8*height/NUMBER_OF_LINES, rendererMain, 
                        2.5*width/NUMBER_OF_COLUMNS, 6*height/NUMBER_OF_LINES);
            textMain = "Sabby, Toby, Damaris, Dennis, Kerstin,";
            renderText(textMain, "lazy.ttf", colorGreen, 0.8*height/NUMBER_OF_LINES, rendererMain, 
                        2.5*width/NUMBER_OF_COLUMNS, 7*height/NUMBER_OF_LINES);
            textMain = "Felix, Simon, Teresa, Simi, Benni,";
            renderText(textMain, "lazy.ttf", colorGreen, 0.8*height/NUMBER_OF_LINES, rendererMain, 
                        2.5*width/NUMBER_OF_COLUMNS, 8*height/NUMBER_OF_LINES);
            textMain = "Thessy, Benni, Ute, Salome, Manu,";
            renderText(textMain, "lazy.ttf", colorGreen, 0.8*height/NUMBER_OF_LINES, rendererMain, 
                        2.5*width/NUMBER_OF_COLUMNS, 9*height/NUMBER_OF_LINES);
            textMain = "David, Sissi, Sarah und Matthias";
            renderText(textMain, "lazy.ttf", colorGreen, 0.8*height/NUMBER_OF_LINES, rendererMain, 
                        2.5*width/NUMBER_OF_COLUMNS, 10*height/NUMBER_OF_LINES);
            */
                        
            //Werbung
            textMain = "(C++ Code frei erhältlich unter https://github.com/MKesenheimer/Familienduell)";
            renderText(textMain, "lazy.ttf", colorGreen, 0.4*height/NUMBER_OF_LINES, rendererMain, 
                        2*width/NUMBER_OF_COLUMNS, 12*height/NUMBER_OF_LINES);
        }
        
        //apply to the screen
        SDL_RenderPresent(rendererTerminal);
        SDL_RenderPresent(rendererMain);
        
        //Timer related stuff
        frame++;
        if (fps.getTicks() < static_cast<int>(1000/FRAMES_PER_SECOND)) {
            SDL_Delay( (1000/FRAMES_PER_SECOND) - fps.getTicks() );
        }
    
        SDL_StopTextInput();
    }

    //destroy the items
	cleanup(rendererMain, windowMain, rendererTerminal, windowTerminal, texture, image);
    Mix_FreeMusic(mix_intro);
    Mix_FreeMusic(mix_wrong);
    Mix_FreeMusic(mix_reveal);
	IMG_Quit();
	SDL_Quit();
    TTF_Quit();
	return 0;
}