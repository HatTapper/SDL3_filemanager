#include "include/SDL_filemanager.h"

static bool shiftPressed = false;

void SDL_DrawBackground(SDL_Application* Application)
{
    SDL_SetRenderDrawColor(Application->Renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(Application->Renderer);

    SDL_SetRenderDrawColor(Application->Renderer, 0x30, 0x30, 0x30, 0xFF);
    SDL_FRect rect = {};
    rect.x = 0;
    rect.y = WINDOW_HEIGHT - 30;
    rect.w = WINDOW_WIDTH;
    rect.h = 30;
    SDL_RenderFillRect(Application->Renderer, &rect);

    SDL_Log("%s", Application->CommandPrompt.text);
    TTF_Text* TTFtext;
    if(Application->CommandPrompt.text != "")
    {
        TTFtext = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, Application->CommandPrompt.text, 0);
    }
    else
    {
        TTFtext = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, "", 0);
    }
    TTF_DrawRendererText(TTFtext, 15, WINDOW_HEIGHT - 24);
    TTF_DestroyText(TTFtext);
}

void SDL_ClearCommandLine(SDL_Application* Application)
{
    for(int i = 0; i < COMMAND_MAX_LENGTH; i++)
    {
        Application->CommandPrompt.text[i] = 0;
    }
}

void SDL_BackspaceCommandLine(SDL_Application* Application)
{
    if(Application->CommandPrompt.text == "")
    {
        return;
    }

    int initialLength = strlen(Application->CommandPrompt.text);
    Application->CommandPrompt.text[initialLength - 1] = 0;
}

void SDL_TypeToCommandLine(SDL_Application* Application, const char* text)
{
    // prevent out of bounds
    int currentTextLength = strlen(Application->CommandPrompt.text);
    if(currentTextLength >= COMMAND_MAX_LENGTH)
    {
        return;
    }

    // append the text to the command prompt
    strncat(Application->CommandPrompt.text, text, strlen(text));
}   

void SDL_ProcessEvent(SDL_Application* Application)
{
    switch(Application->Event.type)
    {
        case SDL_EVENT_QUIT:
        {
            Application->running = false;
        } break;
        case SDL_EVENT_KEY_DOWN:
        {
            if(Application->Event.key.scancode == SDL_SCANCODE_RETURN)
            {
                SDL_ClearCommandLine(Application);
            }
            else if(Application->Event.key.scancode == SDL_SCANCODE_BACKSPACE)
            {
                SDL_BackspaceCommandLine(Application);
            }
        } break;
        case SDL_EVENT_KEY_UP:
        {
        } break;
        case SDL_EVENT_TEXT_INPUT:
        {
            SDL_TypeToCommandLine(Application, Application->Event.text.text);
        } break;
        case SDL_EVENT_MOUSE_MOTION:
        {  
            Application->MouseData.posX = Application->Event.motion.x;
            Application->MouseData.posY = Application->Event.motion.y;
            Application->MouseData.deltaX = Application->Event.motion.xrel;
            Application->MouseData.deltay = Application->Event.motion.yrel;
        } break;
    }
}

void SDL_DrawWindowBackground(SDL_Application* Application)
{
    // first draw the bottom-most color
    SDL_SetRenderDrawColor(Application->Renderer, 0x80, 0x80, 0x80, 0xFF);
    SDL_RenderClear(Application->Renderer);

    // then draw the left-hand list background
    SDL_SetRenderDrawColor(Application->Renderer, 0x90, 0x90, 0x90, 0xFF);
    SDL_FRect rect = {};
    rect.x = 0;
    rect.y = 0;
    rect.w = 180;
    rect.h = WINDOW_HEIGHT;
    SDL_RenderFillRect(Application->Renderer, &rect);

    DIR* directoryStream = opendir("/");
    if(directoryStream == NULL)
    {
        SDL_Log("stream is null");
    }
    else
    {
        dirent* entry;

        int x_offset = 0;
        int y_offset = 0;
        while((entry = readdir(directoryStream)) != NULL)
        {
            TTF_Text* text = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, entry->d_name, 0);
            TTF_DrawRendererText(text, 30 + x_offset, y_offset);
            y_offset += 24;
            if(y_offset > WINDOW_HEIGHT)
            {
                y_offset = 0;
                x_offset += WINDOW_WIDTH / 2;
            }
            TTF_DestroyText(text);
        }
    }
}

int main(void)
{
    // initialization
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL.");
        return SDL_APP_FAILURE;
    }
    if(!TTF_Init())
    {
        SDL_Log("Failed to initialize TTF.");
        return SDL_APP_FAILURE;
    }

    SDL_Application Application;
    SDL_Event Event;
    SDL_MouseData mData = {};
    SDL_FontRenderer mFontRenderer = {};
    SDL_CommandPrompt mCommandPrompt = {};
    mCommandPrompt.shouldFree = false;
    mCommandPrompt.text = (char*)calloc(sizeof(char) * COMMAND_MAX_LENGTH, sizeof(char));
    bool running = true;
    const bool* keyMap = SDL_GetKeyboardState(NULL);

    Uint64 deltaTime = 0;

    Application.Window = SDL_CreateWindow("File Manager", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    Application.Surface = SDL_GetWindowSurface(Application.Window);
    Application.Renderer = SDL_GetRenderer(Application.Window);
    Application.MouseData = mData;
    Application.FontRenderer = mFontRenderer;
    Application.CommandPrompt = mCommandPrompt;
    Application.Event = Event;
    Application.running = true;

    SDL_Rect rect = {};
    rect.x = 0;
    rect.y = WINDOW_HEIGHT - 30;
    rect.w = WINDOW_WIDTH;
    rect.h = 30;
    SDL_SetTextInputArea(Application.Window, &rect, 0);

    Application.FontRenderer.TextEngine = TTF_CreateRendererTextEngine(Application.Renderer);
    Application.FontRenderer.Font = TTF_OpenFont("data/font/FreeSans.ttf", 16);

    if(Application.Surface == NULL)
    {
        SDL_Log("Failed to load window surface");
        return SDL_APP_FAILURE;
    }
    if(Application.Renderer == NULL)
    {
        SDL_Log("Failed load renderer");
        return SDL_APP_FAILURE;
    }

    SDL_StartTextInput(Application.Window);

    // main application loop
    while(Application.running)
    {
        Uint64 currentTick = SDL_GetTicks();
        // event handler
        while(SDL_PollEvent(&Application.Event))
        {
            SDL_ProcessEvent(&Application);
        }

        //SDL_DrawWindowBackground(&Application);
        SDL_DrawBackground(&Application);
        SDL_RenderPresent(Application.Renderer);
        

        Uint64 timeToComplete = SDL_GetTicks() - currentTick;
        // we want a consistent 60fps frame rate
        // this is granular enough for a simple file manager
        Uint64 remaining = GOAL_FRAME_TIME - (double)timeToComplete;
        if(remaining > 0)
        {
            SDL_Delay(remaining);
        }

        deltaTime = SDL_GetTicks() - currentTick;

        #if DEBUG
            SDL_Log("%d FPS | %dms frametime", (int) (1000.0 / deltaTime), deltaTime);
        #endif
    }

    SDL_StopTextInput(Application.Window);

    TTF_CloseFont(Application.FontRenderer.Font);
    TTF_DestroyRendererTextEngine(Application.FontRenderer.TextEngine);
    SDL_DestroyRenderer(Application.Renderer);
    SDL_DestroyWindowSurface(Application.Window);
    SDL_DestroyWindow(Application.Window);
    free(Application.CommandPrompt.text);
    SDL_Quit();   
}