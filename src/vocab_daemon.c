
#define _XOPEN_SOURCE 700

#include "db.h"
#include "eval.h"
#include "paths.h"
#include "sm2.h"
#include "ui.h"
#include "utils.h"
#include "vocab_entry.h"

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// state
typedef struct
{
        unsigned short passive_mode_state;
        // ...
} daemonState;

// macros

// signal handler
volatile sig_atomic_t keep_running = 1;
void handle_sigterm(int signum)
{
    keep_running = 0;
}

// signal handler setup
void setup_signal_handling()
{
    struct sigaction sa;
    sa.sa_handler = handle_sigterm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    signal(SIGCHLD, SIG_IGN);
}

// vocab logic

void check_and_prompt_vocab(daemonState *state)
{
    vocab_entry cur_ve;
    if (get_due_vocab(&cur_ve))
    {
        time_t now = time(NULL);
        char *language = NULL;
        char final_display[512] = {0};
        char *origin_word = NULL;
        char *translation = NULL;
        char final_solution[256] = {0};
        // * create bidirectinoal logic
        if (strchr(cur_ve.front_text, '{') != NULL)
        {
            parse_lueckentext(cur_ve.front_text, final_display, final_solution, sizeof(final_display), sizeof(final_solution));
            language = "de"; // UI-hint
        }
        else if (strchr(cur_ve.back_text, '{') != NULL)
        {
            parse_lueckentext(cur_ve.back_text, final_display, final_solution, sizeof(final_display), sizeof(final_solution));
            language = cur_ve.language; // UI-hint
        }
        else
        {
            int direction = rand() % 2;
            if (direction == 0)
            {
                origin_word = cur_ve.front_text;
                translation = cur_ve.back_text;
                language = cur_ve.language;
            }
            else
            {
                origin_word = cur_ve.back_text;
                translation = cur_ve.front_text;
                language = USER_LANG;
            }
            strncpy(final_display, origin_word, sizeof(final_display));
            strncpy(final_solution, translation, sizeof(final_solution));
        }

        // * check state
        if (state->passive_mode_state)
        {
            ui_display_vocab_passive(final_display, final_solution);
            cur_ve.next_due = now + 300;
        }
        else
        {
            char answer[128] = {0};
            // * create command string for notification
            ui_prompt_translation(final_display, language, answer, sizeof(answer) / sizeof(char));
            time_t done = time(NULL);
            int time_taken = done - now;
            if (answer[0] == '\0')
            {
                cur_ve.next_due = now + 300;
            }
            else
            {
                answer[strcspn(answer, "\r\n")] = '\0';
                int distance = apply_levenshtein(answer, final_solution);
                int lvl = calculate_level(distance, time_taken, strlen(final_solution));
                ui_show_feedback(translation, distance, lvl);
                calculate_sm2(&cur_ve, lvl);
                cur_ve.last_occurence = now;
            }
        }
        update_vocab(&cur_ve);
    }
}

// add vocab logic

void handle_ipc_message(int *fd, char *buffer, int size, daemonState *state)
{

    ssize_t bytes_read = read(*fd, buffer, size - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        char *token = strtok(buffer, "|");
        if (token == NULL)
        {
            // TODO Logging
            return;
        }
        if (strcmp(token, "ADD") == 0)
        {
            vocab_entry ve;
            memset(&ve, 0, sizeof(vocab_entry));

            token = strtok(buffer, "|");
            if (token == NULL)
            {
                // TODO Logging
                return;
            }
            strncpy(ve.language, token, sizeof(ve.language));
            ve.language[sizeof(ve.language) - 1] = '\0';

            token = strtok(NULL, "|");
            if (token == NULL)
            {
                // TODO Logging
                return;
            }
            strncpy(ve.front_text, token, sizeof(ve.front_text));
            ve.front_text[sizeof(ve.front_text) - 1] = '\0';

            token = strtok(NULL, "|");
            if (token == NULL)
            {
                // TODO Logging
                return;
            }
            strncpy(ve.back_text, token, sizeof(ve.back_text));
            ve.back_text[sizeof(ve.back_text) - 1] = '\0';

            token = strtok(NULL, "|");
            if (token == NULL)
            {
                // TODO Logging
                return;
            }
            char *endptr;
            ve.entry_type = strtol(token, &endptr, 10);

            token = strtok(NULL, "|");
            if (token == NULL)
            {
                // TODO Logging
                return;
            }
            strncpy(ve.tags, token, sizeof(ve.tags));
            ve.tags[sizeof(ve.tags) - 1] = '\0';

            ve.ease_factor = 2.5;
            time_t now = time(NULL);
            ve.creation_date = now;
            insert_vocab(&ve);
        }
        else if (strcmp(token, "MODE") == 0)
        {
            token = strtok(NULL, "|");
            if (token == NULL)
            {
                // TODO Logging
                return;
            }
            if (strcmp(token, "PASSIVE") == 0)
                state->passive_mode_state = 1;
            else
                state->passive_mode_state = 0;
        }
        else
        {
            // * Unknown command
        }
    }
    else if (bytes_read == 0)
    {
        close(*fd);
        *fd = open(PIPE_PATH, O_RDWR);
    }
    return;
}

void daemonize()
{
    pid_t PID, w;
    int status;
    printf("Starting Vocab_Trainer Daemon...\n");

    // * create daemon by forking twice and ending parent processes
    PID = fork();
    if (PID == 0)
    {
        // *  child gets own session ID
        PID = setsid();
        // * fork again
        PID = fork();

        if (PID == 0)
        {
            // * switch to root dir and bend stdin/out/err to the void
            chdir("/");
            int null_fd = open("/dev/null", O_RDWR);
            dup2(null_fd, STDIN_FILENO);
            dup2(null_fd, STDOUT_FILENO);
            dup2(null_fd, STDERR_FILENO);
            close(null_fd);
            umask(0); // new created files/dirs wont be masked
            // * create Pipe
            mkfifo(PIPE_PATH, 0666);

            // * open pipe in readonly
            int fd = open(PIPE_PATH, O_RDWR);

            // * create polling struct
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN;
            daemonState state = {.passive_mode_state = 1};

            while (keep_running)
            {
                char buffer[256];
                int ret;

                if (state.passive_mode_state == 0)
                    ret = poll(&pfd, 1, 20000);
                else
                    ret = poll(&pfd, 1, 120000);

                if (ret == 0)
                {
                    // * Timer for polling is over -> prompt for next vocab

                    check_and_prompt_vocab(&state);
                }
                else if (ret < 0)
                {
                    /*
                        * Error -> do nothing for now
                        TODO Implement logging
                    */
                }
                else
                {
                    // * check if FIFO pipe is opened / if new word gets added
                    handle_ipc_message(&fd, buffer, sizeof(buffer), &state);
                }
            }
            close(fd);
            unlink(PIPE_PATH);
        }
        else if (PID > 0)
        {
            // * first child gets ended, second child is now an orphan
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("Error creating child");
        }
    }
    else if (PID > 0)
    {
        // * first parent ends, first child is now son of PID = 1 (systemd)
        exit(EXIT_SUCCESS);
    }
    else
    {
        perror("ERROR creating child..");
    }
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    setup_signal_handling();
    daemonize();

    return 0;
}