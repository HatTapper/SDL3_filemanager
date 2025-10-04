#include "include/SDL_filemanager.h"

static bool shiftPressed = false;

void SDL_ClearCommandLine(SDL_Application* Application)
{
    SDL_FRect clear = {};
    clear.x = 0;
    clear.y = WINDOW_HEIGHT - 30;
    clear.w = WINDOW_WIDTH;
    clear.h = 30;

    SDL_SetRenderDrawColor(Application->Renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(Application->Renderer, &clear);
}

void SDL_TypeToCommandLine(SDL_Application* Application, SDL_KeyboardEvent* event)
{
    static int xOffset = 0;

    SDL_Keycode key = SDL_GetKeyFromScancode(event->scancode, event->mod, true);
    const char* keyName = SDL_GetKeyName(key);
    if(!strncmp(keyName, "Space", 5))
    {
        keyName = " ";
    }
    else if(!strncmp(keyName, "Backspace", 9))
    {
        SDL_ClearCommandLine(Application);
        xOffset = 0;
        return;
    }
    else if(!strncmp(keyName, "Left Shift", 10))
    {
        return;
    }

    if(shiftPressed)
    {
        char* modifiable_str = (char*)malloc(strlen(keyName) + 1);
        strncpy(modifiable_str, keyName, strlen(keyName));
        keyName = SDL_strupr(modifiable_str);
    }

    int textWidth = 0;
    TTF_Text* text = TTF_CreateText(Application->FontRenderer.TextEngine, Application->FontRenderer.Font, keyName, 0);
    TTF_DrawRendererText(text, 30 + xOffset, WINDOW_HEIGHT - 30);
    TTF_GetTextSize(text, &textWidth, NULL);
    TTF_DestroyText(text);

    xOffset += textWidth;
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
            SDL_Log("Key down. %c", Application->Event.key.key);
            const char* keyName = SDL_GetKeyName(Application->Event.key.key);
            if(!strncmp(keyName, "Left Shift", 10))
            {
                shiftPressed = true;
            }
            SDL_TypeToCommandLine(Application, &Application->Event.key);
        } break;
        case SDL_EVENT_KEY_UP:
        {
            SDL_Log("Key up. %c", Application->Event.key.key);
            const char* keyName = SDL_GetKeyName(Application->Event.key.key);
            if(!strncmp(keyName, "Left Shift", 10))
            {
                shiftPressed = false;
            }
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
    bool running = true;
    const bool* keyMap = SDL_GetKeyboardState(NULL);

    Uint64 deltaTime = 0;

    Application.Window = SDL_CreateWindow("File Manager", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    Application.Surface = SDL_GetWindowSurface(Application.Window);
    Application.Renderer = SDL_GetRenderer(Application.Window);
    Application.MouseData = mData;
    Application.FontRenderer = mFontRenderer;
    Application.Event = Event;
    Application.running = true;

    Application.FontRenderer.TextEngine = TTF_CreateRendererTextEngine(Application.Renderer);
    Application.FontRenderer.Font = TTF_OpenFont("data/font/Ubuntu-L.ttf", 16);

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
        SDL_RenderPresent(Application.Renderer);
        

        Uint64 timeToComplete = SDL_GetTicks() - currentTick;
        // we want a consistent 60fps frame rate
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

    TTF_CloseFont(Application.FontRenderer.Font);
    TTF_DestroyRendererTextEngine(Application.FontRenderer.TextEngine);
    SDL_DestroyRenderer(Application.Renderer);
    SDL_DestroyWindowSurface(Application.Window);
    SDL_DestroyWindow(Application.Window);
    SDL_Quit();   
}