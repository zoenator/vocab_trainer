#include "ui.h"

void ui_prompt_translation(char *origin_word, char *language, char *answer_buffer, size_t answer_size)
{
    char cmd[512];
    snprintf(
        cmd, sizeof(cmd), "notify-send --action='default=Antworten' -t 10000 'Vocab Trainer' 'Translate: %s to %s'", origin_word, language);

    char action[64] = {0};
    FILE *fp = popen(cmd, "r");
    fgets(action, sizeof(action), fp);
    pclose(fp);
    if (strncmp(action, "default", strlen("default")) == 0)
    {
        // * create command string for zenity prompt
        snprintf(cmd, sizeof(cmd), "zenity --entry --title='Vocab Trainer' --text='Was heißt %s auf %s?'", origin_word, language);
        FILE *fp = popen(cmd, "r");
        char answer[128];
        fgets(answer, sizeof(answer), fp);
        pclose(fp);
        answer[strcspn(answer, "\r\n")] = '\0';
        strcpy(answer_buffer, answer);
    }
    else
    {
        answer_buffer[0] = '\0';
    }
}