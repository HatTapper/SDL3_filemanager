#pragma once

#ifndef SDL_FILEMANAGER_H
#define SDL_FILEMANAGER_H


#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#define GOAL_FRAME_RATE 60
#define GOAL_FRAME_TIME (1000.0 / (double)GOAL_FRAME_RATE)
#define DEBUG 0
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 540
#define COMMAND_MAX_LENGTH 100

struct SDL_MouseData {
    int posX;
    int posY;
    int deltaX;
    int deltay;
};

struct SDL_FontRenderer {
    TTF_TextEngine* TextEngine;
    TTF_Font* Font;
};

struct SDL_CommandPrompt {
    char text[COMMAND_MAX_LENGTH];
    bool shouldFree;
};

struct StringArray {
    char* str;
    size_t length;
    StringArray* next;
};

struct DirectoryData {
    char focusedFile[PATH_MAX];
};

struct SDL_Application {
    SDL_Window* Window;
    SDL_Surface* Surface;
    SDL_Renderer* Renderer;
    SDL_FontRenderer FontRenderer;
    SDL_MouseData MouseData;
    SDL_CommandPrompt CommandPrompt;
    DirectoryData DirData;
    
    bool running;
};



// these are functions that are necessary to handle the command input
void CMD_HandleCommand(SDL_Application* Application, char* textSequence);
void CMD_ClearCommandLine(SDL_Application* Application);
void CMD_BackspaceCommandLine(SDL_Application* Application);
void CMD_TypeToCommandLine(SDL_Application* Application, const char* text);

#endif