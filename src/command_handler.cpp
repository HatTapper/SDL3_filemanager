#include "include/SDL_filemanager.h"

void CMD_ClearCommandLine(SDL_Application* Application)
{
    for(int i = 0; i < COMMAND_MAX_LENGTH; i++)
    {
        Application->CommandPrompt.text[i] = 0;
    }
}

void CMD_BackspaceCommandLine(SDL_Application* Application)
{
    if(Application->CommandPrompt.text == "")
    {
        return;
    }

    int initialLength = strlen(Application->CommandPrompt.text);
    Application->CommandPrompt.text[initialLength - 1] = 0;
}

void CMD_TypeToCommandLine(SDL_Application* Application, const char* text)
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

void CMD_HandleCommand(SDL_Application* Application, char* textSequence)
{
    SDL_Log("I was successfully called!");
    return;
}