#ifndef SDL_FILEMANAGER_H

#define SDL_FILEMANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_main.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#define GOAL_FRAME_RATE 60
#define GOAL_FRAME_TIME ((double)1000 / (double)GOAL_FRAME_RATE)
#define DEBUG 0
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 540

typedef struct SDL_MouseData {
    int posX;
    int posY;
    int deltaX;
    int deltay;
} SDL_MouseData;

typedef struct SDL_FontRenderer {
    TTF_TextEngine* TextEngine;
    TTF_Font* Font;
} SDL_FontRenderer;

typedef struct SDL_Application {
    SDL_Window* Window;
    SDL_Surface* Surface;
    SDL_Renderer* Renderer;
    SDL_FontRenderer FontRenderer;
    SDL_MouseData MouseData;
    SDL_Event Event;
    
    bool running;
} SDL_Application;

typedef struct Directory {
    char fileName[PATH_MAX];
    int numChildren;
    Directory* children;
} Directory;

#endif