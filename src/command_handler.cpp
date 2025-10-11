#include "include/SDL_filemanager.h"

enum CommandType {
    COMMAND_CD,
    COMMAND_MKDIR,
    COMMAND_QUIT,
    COMMAND_UNKNOWN,
};

int getCommandEnum(const char* command)
{
    int commandLength = strlen(command);
    if(SDL_strncmp(command, "cd", commandLength) == 0)
    {
        return COMMAND_CD;
    }
    else if(SDL_strncmp(command, "mkdir", commandLength) == 0)
    {
        return COMMAND_MKDIR;
    }
    else if(SDL_strncmp(command, "quit", commandLength) == 0)
    {
        return COMMAND_QUIT;
    }

    return COMMAND_UNKNOWN;
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
    char* chrptr;
    char* cmd = SDL_strtok_r(textSequence, " ", &chrptr);
    int commandEnum = getCommandEnum(cmd);
    if(commandEnum == COMMAND_UNKNOWN)
    {
        SDL_Log("Invalid command provided.");
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
            int newDirectoryLength = strlen(newDirectory);

            SDL_strlcpy(Application->DirData.focusedFile, newDirectory, newDirectoryLength + 1);
            SDL_Log(Application->DirData.focusedFile);
        } break;
        case COMMAND_MKDIR:
        {
            char* newDirectoryName = parameterList[0];
            int newDirectoryLength = strlen(newDirectoryName);
            SDL_Log(newDirectoryName);
            
            char dirName[PATH_MAX];
            SDL_snprintf(dirName, PATH_MAX, "%s/%s", Application->DirData.focusedFile, newDirectoryName);
            
            int check = mkdir(dirName);
            if(!check)
            {
                SDL_Log("Folder created successfully.");
            }
            else
            {
                SDL_Log("Error creating file: %s", dirName);
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