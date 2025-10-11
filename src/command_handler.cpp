#include "include/SDL_filemanager.h"

enum CommandType {
    COMMAND_CD, // expects one argument, sets the focused path to the first argument 
    COMMAND_MKDIR, // expects one argument, creates a new directory at the focused path and sets its name to the first argument
    COMMAND_QUIT, // takes zero arguments, quits the program
    COMMAND_UNKNOWN, // this is for if the user provides a command that doesn't exist, should be handled by the program
};

int getCommandEnum(const char* command)
{
    int commandLength = strlen(command);
    if(SDL_strncmp(command, "cd", 3) == 0)
    {
        return COMMAND_CD;
    }
    else if(SDL_strncmp(command, "mkdir", 6) == 0)
    {
        return COMMAND_MKDIR;
    }
    else if(SDL_strncmp(command, "quit", 5) == 0)
    {
        return COMMAND_QUIT;
    }

    return COMMAND_UNKNOWN;
}

void CMD_WriteToOutput(SDL_Application* Application, const char* text, Uint32 rgba)
{
    SDL_strlcpy(Application->CommandPrompt.outputText, text, 128);

    ColorData clrData = {};
    clrData.r = (rgba >> 0) & 0xFF;
    clrData.g = (rgba >> 8) & 0xFF;
    clrData.b = (rgba >> 16) & 0xFF; 
    clrData.a = (rgba >> 24) & 0xFF;
    Application->CommandPrompt.outputTextColor = clrData;
}

void CMD_ClearCommandLine(SDL_Application* Application)
{
    for(int i = 0; i < COMMAND_MAX_LENGTH; i++)
    {
        Application->CommandPrompt.text[i] = 0;
    }
}

void CMD_BackspaceCommandLine(SDL_Application* Application)
{
    if(strlen(Application->CommandPrompt.text) == 0)
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
    char* chrptr;
    char* cmd = SDL_strtok_r(textSequence, " ", &chrptr);
    int commandEnum = getCommandEnum(cmd);
    if(commandEnum == COMMAND_UNKNOWN)
    {
        CMD_WriteToOutput(Application, "Invalid command provided.", 0xFF0000FF);
        return;
    }

    char** parameterList = (char**)malloc(sizeof(char*) * 8);
    int i = 0;
    char* parameter;
    while((parameter = SDL_strtok_r(NULL, " ", &chrptr)) != NULL)
    {
        parameterList[i++] = parameter;
    } 

    switch(commandEnum)
    {
        case COMMAND_CD:
        {
            char* newDirectory = parameterList[0];
            if(newDirectory == NULL)
            {
                CMD_WriteToOutput(Application, "cd requires an argument describing the path.", 0xFF0000FF);
            }
            else
            {
                int newDirectoryLength = strlen(newDirectory);

                SDL_strlcpy(Application->DirData.focusedFile, newDirectory, newDirectoryLength + 1);
                SDL_Log(Application->DirData.focusedFile);
            }
        } break;
        case COMMAND_MKDIR:
        {
            char* newDirectoryName = parameterList[0];
            if(newDirectoryName == NULL)
            {
                CMD_WriteToOutput(Application, "mkdir requires an argument describing the new directory name.", 0xFF0000FF);
            }
            int newDirectoryLength = strlen(newDirectoryName);
            SDL_Log(newDirectoryName);
            
            char dirName[PATH_MAX];
            SDL_snprintf(dirName, PATH_MAX, "%s/%s", Application->DirData.focusedFile, newDirectoryName);
            
            int check = mkdir(dirName);
            if(!check)
            {
                CMD_WriteToOutput(Application, "Folder created successfully.", 0x00FF00FF);
            }
            else
            {
                char buffer[384];
                SDL_snprintf(buffer, 384, "Error creating directory: %s", dirName);
                CMD_WriteToOutput(Application, buffer, 0xFF0000FF);
            }
        } break;
        case COMMAND_QUIT:
        {
            Application->running = false;
        } break;
    }

    free(parameterList);
    
    return;
}