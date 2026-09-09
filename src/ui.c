#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void ui_display_vocab_passive(char *front_text, char *back_text)
{
    char text[512];
    snprintf(text, sizeof(text), "%s | %s", front_text, back_text);
    if (fork() == 0)
    {
        execlp("notify-send", "notify-send", "-t", "10000", "-h", "int:transient:1", "Vocab Trainer", text, NULL);
        exit(1);
    }
    return;
}

void ui_prompt_translation(char *origin_word, char *language, char *answer_cmd, size_t answer_size)
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
        strcpy(answer_cmd, answer);
    }
    else
    {
        answer_cmd[0] = '\0';
    }
}

void ui_show_feedback(const char *correct_word, int distance, int level)
{
    const char *icon;
    const char *urgency;
    char text[128];
    switch (level)
    {

    case 5:
        urgency = "low";
        icon = "/usr/share/icons/Qogir/scalable/apps/butter.svg";
        snprintf(text, sizeof(text), "Correct. <<%s>>", correct_word);
        break;
    case 4:
    case 3:
        urgency = "normal";
        icon = "/usr/share/icons/Papirus/64x64/apps/steam_icon_481510.svg";
        snprintf(text, sizeof(text), "Almost correct. Solution: <<%s>>", correct_word);
        break;
    case 2:
    case 1:
    case 0:
        urgency = "critical";
        icon = "/usr/share/icons/Papirus/64x64/apps/clanbomber.svg";
        snprintf(text, sizeof(text), "Incorrect, the right solution is <<%s>>", correct_word);
        break;
    default:
        snprintf(text, sizeof(text), "Error!");
        break;
    }

    if (fork() == 0)
    {
        execlp("notify-send", "notify-send", "-u", urgency, "-i", icon, "Rating", text, NULL);
        exit(1);
    }
    return;
}