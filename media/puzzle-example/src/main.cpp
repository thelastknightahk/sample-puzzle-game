#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include "PuzzleLogic.h"


static void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    if (!font || text.empty()) return;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        printf("TTF_RenderText_Blended failed: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

static void writeLog(const std::string& filename, const std::string& message) {
    std::ofstream logFile(filename, std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}

int main(int argc, char* args[]) {
    // Get base path
    char* basePath = SDL_GetBasePath();
    std::string logPath = "puzzle_log.txt";
    std::string errorPath = "error_log.txt";
    
    if (basePath) {
        logPath = std::string(basePath) + "puzzle_log.txt";
        errorPath = std::string(basePath) + "error_log.txt";
        SDL_free(basePath);
    }

    // Clear old logs
    std::ofstream(logPath, std::ios::trunc).close();
    std::ofstream(errorPath, std::ios::trunc).close();

    writeLog(logPath, "=== Puzzle Game Starting ===");
    writeLog(logPath, "Base path: " + std::string(basePath ? basePath : "NULL"));

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) {
        std::string err = "SDL could not initialize! SDL_Error: " + std::string(SDL_GetError());
        writeLog(errorPath, err);
        fprintf(stderr, "%s\n", err.c_str());
        return -1;
    }
    writeLog(logPath, "SDL initialized successfully");

    // Create window
    SDL_Window* window = SDL_CreateWindow("Puzzle Prime Aung Htet Kyaw",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        std::string err = "Window could not be created! SDL_Error: " + std::string(SDL_GetError());
        writeLog(errorPath, err);
        fprintf(stderr, "%s\n", err.c_str());
        SDL_Quit();
        return -1;
    }
    writeLog(logPath, "Window created successfully");

    // Create renderer - try hardware first, fall back to software
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!renderer) {
        writeLog(logPath, "Hardware renderer failed, trying software...");
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!renderer) {
        std::string err = "Renderer could not be created! SDL_Error: " + std::string(SDL_GetError());
        writeLog(errorPath, err);
        fprintf(stderr, "%s\n", err.c_str());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Log renderer info
    SDL_RendererInfo rinfo;
    if (SDL_GetRendererInfo(renderer, &rinfo) == 0) {
        writeLog(logPath, std::string("Renderer: ") + rinfo.name);
        writeLog(logPath, "Renderer flags: " + std::to_string(rinfo.flags));
    }

    // Initialize TTF
    if (TTF_Init() < 0) {
        std::string err = "TTF_Init failed: " + std::string(TTF_GetError());
        writeLog(errorPath, err);
        printf("%s\n", err.c_str());
    } else {
        writeLog(logPath, "SDL_ttf initialized");
    }

    // Try multiple font paths
    TTF_Font* font = nullptr;
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };

    for (const char* path : fontPaths) {
        font = TTF_OpenFont(path, 24);
        if (font) {
            writeLog(logPath, std::string("Font loaded: ") + path);
            break;
        }
    }

    if (!font) {
        writeLog(errorPath, "Could not load any font!");
        printf("Warning: No font loaded - text will not display\n");
    }

    // Detect input devices
    SDL_GameController* controller = nullptr;
    SDL_Joystick* joystick = nullptr;

    writeLog(logPath, "Checking for input devices...");
    int numJoysticks = SDL_NumJoysticks();
    writeLog(logPath, "Number of joysticks: " + std::to_string(numJoysticks));

    for (int i = 0; i < numJoysticks; i++) {
        const char* name = SDL_JoystickNameForIndex(i);
        writeLog(logPath, std::string("Joystick ") + std::to_string(i) + ": " + (name ? name : "Unknown"));
        
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                writeLog(logPath, std::string("Game controller opened: ") + SDL_GameControllerName(controller));
                printf("Controller: %s\n", SDL_GameControllerName(controller));
            }
        } else {
            joystick = SDL_JoystickOpen(i);
            if (joystick) {
                writeLog(logPath, std::string("Joystick opened: ") + SDL_JoystickName(joystick));
                writeLog(logPath, "Buttons: " + std::to_string(SDL_JoystickNumButtons(joystick)));
                writeLog(logPath, "Axes: " + std::to_string(SDL_JoystickNumAxes(joystick)));
                printf("Joystick: %s (Buttons: %d, Axes: %d)\n", 
                       SDL_JoystickName(joystick),
                       SDL_JoystickNumButtons(joystick),
                       SDL_JoystickNumAxes(joystick));
            }
        }
    }

    if (!controller && !joystick) {
        writeLog(logPath, "No game controller or joystick detected");
        printf("No controller/joystick detected - using keyboard only\n");
    }

    // Initialize game
    PuzzleLogic game(3, 3);
    bool running = true;
    std::string lastInput = "Press any key/button to test";
    bool showInputPanel = true;

    writeLog(logPath, "Game initialized, starting main loop");
    printf("Game started! Check puzzle_log.txt for details\n");

    auto moveBlank = [&](int dx, int dy) {
        int emptyX = 0, emptyY = 0;
        game.getEmptyPosition(emptyX, emptyY);
        game.moveTile(emptyX + dx, emptyY + dy);
    };

    auto handleMove = [&](int dx, int dy, const char* label) {
        moveBlank(dx, dy);
        lastInput = label;
        writeLog(logPath, std::string("Input: ") + label);
    };

    // Main loop
    Uint32 frameStart = 0;
    int frameTime = 16; // ~60 FPS

    while(running) {
        frameStart = SDL_GetTicks();

        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                writeLog(logPath, "Quit event received");
            }
            else if (e.type == SDL_KEYDOWN) {
                printf("Key pressed: %s (code: %d)\n", SDL_GetKeyName(e.key.keysym.sym), e.key.keysym.sym);
                writeLog(logPath, std::string("Key: ") + SDL_GetKeyName(e.key.keysym.sym));

                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_UP:
                    case SDLK_w:
                        handleMove(0, -1, "Keyboard: UP");
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        handleMove(0, 1, "Keyboard: DOWN");
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        handleMove(-1, 0, "Keyboard: LEFT");
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        handleMove(1, 0, "Keyboard: RIGHT");
                        break;
                    case SDLK_r:
                        game = PuzzleLogic(3, 3);
                        lastInput = "Keyboard: R (reset)";
                        break;
                }
            }
            else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                printf("Controller button: %d\n", e.cbutton.button);
                writeLog(logPath, std::string("Controller button: ") + std::to_string(e.cbutton.button));

                switch (e.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        handleMove(0, -1, "Controller: DPAD UP");   
                        break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        handleMove(0, 1, "Controller: DPAD DOWN");   
                        break;
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                        handleMove(-1, 0, "Controller: DPAD LEFT");  
                        break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                        handleMove(1, 0, "Controller: DPAD RIGHT");  
                        break;
                    case SDL_CONTROLLER_BUTTON_A:
                        handleMove(0, -1, "Controller: A");  
                        break;
                    case SDL_CONTROLLER_BUTTON_B:
                        handleMove(0, 1, "Controller: B");   
                        break;
                    case SDL_CONTROLLER_BUTTON_X:
                        handleMove(-1, 0, "Controller: X"); 
                        break;
                    case SDL_CONTROLLER_BUTTON_Y:
                        handleMove(1, 0, "Controller: Y");  
                        break;
                    case SDL_CONTROLLER_BUTTON_START:
                        game = PuzzleLogic(3, 3);
                        lastInput = "Controller: START (reset)";
                        break;
                    case SDL_CONTROLLER_BUTTON_BACK:
                        running = false;
                        break;
                }
            }
            else if (e.type == SDL_JOYBUTTONDOWN) {
                printf("Joystick button pressed: %d\n", e.jbutton.button);
                writeLog(logPath, std::string("Joystick button: ") + std::to_string(e.jbutton.button));
                
                // Map first 4 buttons to directions
                switch(e.jbutton.button) {
                    case 0: handleMove(0, -1, "Joystick Button 0"); break;  // Changed from 0, 1
                    case 1: handleMove(0, 1, "Joystick Button 1"); break;   // Changed from 0, -1
                    case 2: handleMove(-1, 0, "Joystick Button 2"); break;  // Changed from 1, 0
                    case 3: handleMove(1, 0, "Joystick Button 3"); break;   // Changed from -1, 0
                    case 4: game = PuzzleLogic(3, 3); lastInput = "Joystick Button 4 (reset)"; break;
                }
            }
            else if (e.type == SDL_JOYAXISMOTION) {
                if (e.jaxis.axis == 1) { // Vertical axis
                    if (e.jaxis.value < -16384) {
                        handleMove(0, -1, "Joystick Axis UP");  // Changed from 0, 1
                    } else if (e.jaxis.value > 16384) {
                        handleMove(0, 1, "Joystick Axis DOWN");  // Changed from 0, -1
                    }
                } else if (e.jaxis.axis == 0) { // Horizontal axis
                    if (e.jaxis.value < -16384) {
                        handleMove(-1, 0, "Joystick Axis LEFT");  // Changed from 1, 0
                    } else if (e.jaxis.value > 16384) {
                        handleMove(1, 0, "Joystick Axis RIGHT");  // Changed from -1, 0
                    }
                }
            }
            else if (e.type == SDL_CONTROLLERDEVICEADDED) {
                if (!controller && SDL_IsGameController(e.cdevice.which)) {
                    controller = SDL_GameControllerOpen(e.cdevice.which);
                    if (controller) {
                        lastInput = "Controller connected";
                        writeLog(logPath, std::string("Controller connected: ") + SDL_GameControllerName(controller));
                    }
                }
            }
            else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
                if (controller && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)) == e.cdevice.which) {
                    SDL_GameControllerClose(controller);
                    controller = nullptr;
                    lastInput = "Controller disconnected";
                }
            }
        }

        // Clear screen - Blue background
        SDL_SetRenderDrawColor(renderer, 30, 60, 120, 255);
        SDL_RenderClear(renderer);

        // Draw input panel
        if (showInputPanel) {
            SDL_Rect panel = {20, 20, 520, 160};
            SDL_SetRenderDrawColor(renderer, 20, 20, 40, 240);
            SDL_RenderFillRect(renderer, &panel);
            SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
            SDL_RenderDrawRect(renderer, &panel);

            if (font) {
                drawText(renderer, font, "=== INPUT TEST MODE ===", 30, 30, {255, 255, 0, 255});
                drawText(renderer, font, "Arrow Keys / WASD = Move", 30, 60, {255, 255, 255, 255});
                drawText(renderer, font, "R = Reset game", 30, 85, {255, 255, 255, 255});
                drawText(renderer, font, "ESC = Exit", 30, 110, {255, 100, 100, 255});
                drawText(renderer, font, ("Last: " + lastInput).c_str(), 30, 140, {0, 255, 0, 255});
            }
        }

        // Draw puzzle board
        const int boardWidth = 3;
        const int boardHeight = 3;
        const int tileSize = 180;
        const int tileGap = 10;
        const int boardX = 600;
        const int boardY = 100;

        // Draw board background
        SDL_Rect boardBg = {boardX - 10, boardY - 10, 
                           boardWidth * (tileSize + tileGap) + 20, 
                           boardHeight * (tileSize + tileGap) + 20};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &boardBg);

        // Draw tiles
        for (int row = 0; row < boardHeight; ++row) {
            for (int col = 0; col < boardWidth; ++col) {
                int value = game.getTile(col, row);
                SDL_Rect box = {
                    boardX + col * (tileSize + tileGap),
                    boardY + row * (tileSize + tileGap),
                    tileSize,
                    tileSize
                };

                if (value == 0) {
                    // Empty tile - dark gray
                    SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
                } else {
                    // Numbered tile - gradient blue
                    SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
                }
                SDL_RenderFillRect(renderer, &box);

                // Draw border
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &box);

                // Draw number
                if (value != 0 && font) {
                    std::string tileText = std::to_string(value);
                    int textWidth = tileSize / 3;
                    int textHeight = tileSize / 3;
                    int textX = box.x + (tileSize - textWidth) / 2;
                    int textY = box.y + (tileSize - textHeight) / 2;
                    drawText(renderer, font, tileText, textX, textY, {0, 0, 0, 255});
                }
            }
        }

        // Draw instructions
        if (font) {
            drawText(renderer, font, "PUZZLE GAME", 650, 50, {255, 255, 0, 255});
            drawText(renderer, font, "Arrange numbers 1-8 in order", 620, 680, {200, 200, 200, 255});
        }

        SDL_RenderPresent(renderer);

        // Frame timing
        Uint32 frameTime_actual = SDL_GetTicks() - frameStart;
        if (frameTime_actual < frameTime) {
            SDL_Delay(frameTime - frameTime_actual);
        }
    }

    // Cleanup
    writeLog(logPath, "Shutting down...");
    
    if (font) TTF_CloseFont(font);
    if (TTF_WasInit()) TTF_Quit();
    
    if (controller) SDL_GameControllerClose(controller);
    if (joystick) SDL_JoystickClose(joystick);
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    writeLog(logPath, "Game exited normally");
    printf("Game closed. Check puzzle_log.txt for details.\n");
    
    return 0;
}