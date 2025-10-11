#include "include/SDL_filemanager.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

void SDL_DrawBackground(SDL_Application* Application)
{
    SDL_SetRenderDrawColor(Application->Renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(Application->Renderer);

    // first draw the bottom-most color
    SDL_SetRenderDrawColor(Application->Renderer, 0x80, 0x80, 0x80, 0xFF);

    // draws the file list
    DirectoryData data = Application->DirData;
    DIR* directoryStream = opendir(data.focusedFile);
    if(directoryStream == NULL)
    {
        TTF_Text* text = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, "There was an error getting the files from this directory.", 0);
        TTF_DrawRendererText(text, 30, 30);
        TTF_DestroyText(text);
    }
    else
    {
        dirent* entry;

        int x_offset = 0;
        int y_offset = 30;
        while((entry = readdir(directoryStream)) != NULL)
        {
            TTF_Text* text = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, entry->d_name, 0);
            TTF_DrawRendererText(text, 30 + x_offset, y_offset);
            y_offset += 24;
            if(y_offset > WINDOW_HEIGHT - 48)
            {
                y_offset = 30;
                x_offset += WINDOW_WIDTH / 3;
            }
            TTF_DestroyText(text);
        }
    }
    closedir(directoryStream);

    // draws the command line at the bottom
    SDL_SetRenderDrawColor(Application->Renderer, 0x30, 0x30, 0x30, 0xFF);
    SDL_FRect rect = {};
    rect.x = 0;
    rect.y = WINDOW_HEIGHT - 30;
    rect.w = WINDOW_WIDTH;
    rect.h = 30;
    SDL_RenderFillRect(Application->Renderer, &rect);

    TTF_Text* TTFtext;
    TTFtext = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, Application->CommandPrompt.text, 0);
    TTF_DrawRendererText(TTFtext, 15, WINDOW_HEIGHT - 24);
    TTF_DestroyText(TTFtext);

    // draws the output line at the top right
    SDL_SetRenderDrawColor(Application->Renderer, 0x20, 0x20, 0x20, 0xFF);
    SDL_FRect topRightRect;
    topRightRect.x = 0;
    topRightRect.y = 0;
    topRightRect.h = 20;
    topRightRect.w = WINDOW_WIDTH;

    SDL_RenderFillRect(Application->Renderer, &topRightRect);
    TTF_Text* TTFText = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, Application->CommandPrompt.outputText, 0);

    ColorData clrData = Application->CommandPrompt.outputTextColor;
    TTF_SetTextColor(TTFText, clrData.r, clrData.g, clrData.b, clrData.a);
    TTF_DrawRendererText(TTFText, 15, 4);
    TTF_DestroyText(TTFText);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    // initialization
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("Failed to initialize SDL.");
        return SDL_APP_FAILURE;
    }
    if(!TTF_Init())
    {
        SDL_Log("Failed to initialize TTF.");
        return SDL_APP_FAILURE;
    }

    SDL_Application* Application = (SDL_Application*) malloc(sizeof(SDL_Application));
    memset(Application, 0, sizeof(SDL_Application));
    if(Application == NULL)
    {
        SDL_Log("Failed to allocate memory for Application");
        return SDL_APP_FAILURE;
    }
    SDL_MouseData mData = {};
    SDL_FontRenderer mFontRenderer = {};
    DirectoryData mDirData = {};
    mDirData.focusedFile[0] = '/';
    mDirData.focusedFile[1] = '\0';
    SDL_CommandPrompt mCommandPrompt = {};
    const bool* keyMap = SDL_GetKeyboardState(NULL);

    Uint64 deltaTime = 0;

    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    //SDL_CreateWindowAndRenderer("File Manager", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &mWindow, &mRenderer); 
    mWindow = SDL_CreateWindow("File Manager", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    mRenderer = SDL_CreateRenderer(mWindow, "opengl");
    Application->Window = mWindow;
    if(Application->Window == NULL)
    {
        SDL_Log("Failed to create window.");
        return SDL_APP_FAILURE;
    }
    Application->Surface = SDL_GetWindowSurface(Application->Window);
    if(Application->Surface == NULL)
    {
        SDL_Log("Failed to load window surface");
        return SDL_APP_FAILURE;
    }
    Application->Renderer = mRenderer;
    if(Application->Renderer == NULL)
    {
        SDL_Log("Failed to load renderer.");
        return SDL_APP_FAILURE;
    } 
    Application->MouseData = mData;
    Application->FontRenderer = mFontRenderer;
    Application->CommandPrompt = mCommandPrompt;
    Application->DirData = mDirData;
    Application->running = true;

    SDL_Rect rect = {};
    rect.x = 0;
    rect.y = WINDOW_HEIGHT - 30;
    rect.w = WINDOW_WIDTH;
    rect.h = 30;
    SDL_SetTextInputArea(Application->Window, &rect, 0);

    Application->FontRenderer.TextEngine = TTF_CreateRendererTextEngine(Application->Renderer);
    Application->FontRenderer.Font = TTF_OpenFont("data/font/FreeSans.ttf", 16);

    if(Application->Surface == NULL)
    {
        SDL_Log("Failed to load window surface");
        return SDL_APP_FAILURE;
    }
    if(Application->Renderer == NULL)
    {
        SDL_Log("Failed load renderer");
        return SDL_APP_FAILURE;
    }

    bool success = SDL_StartTextInput(Application->Window);
    if(!success)
    {
        SDL_Log("Failed to start text input on the window.");
        return SDL_APP_FAILURE;
    }

    // set "global" application variable
    // void pointers sure are cool!
    *appstate = Application;

    CMD_WriteToOutput(Application, "Welcome! Program successfully started.", 0xFFFFFFFF);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_Application* Application = (SDL_Application*) appstate;
    if(!Application->running)
    {
        return SDL_APP_SUCCESS;
    }

    Uint64 currentTick = SDL_GetTicks();

    SDL_RenderClear(Application->Renderer);
    SDL_DrawBackground(Application);
        
    SDL_RenderPresent(Application->Renderer);

    Uint64 timeToComplete = SDL_GetTicks() - currentTick;
    // we want a consistent 60fps frame rate
    // this is granular enough for a simple file manager
    Uint64 remaining = GOAL_FRAME_TIME - (double)timeToComplete;
    if(remaining > 0)
    {
        SDL_Delay(remaining);
    }

    int deltaTime = SDL_GetTicks() - currentTick;

    #if DEBUG
        SDL_Log("%d FPS | %dms frametime", (int) (1000.0 / deltaTime), deltaTime);
    #endif

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    SDL_Application* Application = (SDL_Application*) appstate;

    switch(event->type)
    {
        // fall through
        // there are some cases where it for some reason
        // opens multiple windows on startup, so this allows them to
        // still close as intended when the X button is pressed.
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_QUIT:
        {
            return SDL_APP_SUCCESS;
        } break;

        case SDL_EVENT_KEY_DOWN:
        {
            if(event->key.scancode == SDL_SCANCODE_RETURN)
            {
                CMD_HandleCommand(Application, Application->CommandPrompt.text);
                CMD_ClearCommandLine(Application);
            }
            else if(event->key.scancode == SDL_SCANCODE_BACKSPACE)
            {
                CMD_BackspaceCommandLine(Application);
            }
        } break;
        case SDL_EVENT_KEY_UP:
        {
        } break;
        case SDL_EVENT_TEXT_INPUT:
        {
            CMD_TypeToCommandLine(Application, event->text.text);
        } break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_Application* Application = (SDL_Application*)appstate;

    SDL_StopTextInput(Application->Window);

    TTF_CloseFont(Application->FontRenderer.Font);
    TTF_DestroyRendererTextEngine(Application->FontRenderer.TextEngine);
    SDL_DestroyWindowSurface(Application->Window);
    SDL_DestroyRenderer(Application->Renderer);
    SDL_DestroyWindow(Application->Window);
    SDL_free(Application);
    SDL_PumpEvents();
    SDL_Quit();

    SDL_Log("Cleanup successful!");
}