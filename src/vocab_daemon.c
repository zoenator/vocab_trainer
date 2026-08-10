#include <stddef.h>
#include <stdint.h>
#include <time.h>
#define _XOPEN_SOURCE 700

#include "db.h"
#include "paths.h"
#include "ui.h"
#include "vocab_entry.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// macros
#define PENALTY_DELAY 60
#define SEC_PER_DAY 86400
#define USER_LANG "DE"

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
}

// vocab logic

void check_and_prompt_vocab()
{
    vocab_entry cur_ve;
    if (get_due_vocab(&cur_ve))
    {
        time_t now = time(NULL);
        // * create bidirectinoal logic
        int direction = rand() % 2;
        char *origin_word = NULL;
        char *translation = NULL;
        char *language = NULL;
        if (direction == 0)
        {
            origin_word = cur_ve.german;
            translation = cur_ve.translation;
            language = cur_ve.language;
        }
        else
        {
            origin_word = cur_ve.translation;
            translation = cur_ve.german;
            language = USER_LANG;
        }

        char answer[128] = {0};
        // * create command string for notification
        ui_prompt_translation(origin_word, language, answer, sizeof(answer) / sizeof(char));
        if (answer[0] == '\0')
        {
            cur_ve.next_due = now + 300;
        }
        else
        {
            answer[strcspn(answer, "\r\n")] = '\0';
            if (strcmp(translation, answer) == 0)
            {
                // * correct answer
                cur_ve.difficulty < 4 ? cur_ve.difficulty++ : cur_ve.difficulty;
                cur_ve.next_due = now + cur_ve.difficulty * SEC_PER_DAY;
            }
            else
            {
                // * incorrect answer
                /*
                    ! rn correct answer has no margin its either right or wrong, implement system that can evaluate
                    ! how close u were to the real solution
                */
                cur_ve.difficulty > 0 ? cur_ve.difficulty-- : cur_ve.difficulty;
                cur_ve.next_due = now + PENALTY_DELAY;
            }
            cur_ve.last_occurence = now;
        }
        update_vocab(&cur_ve);
    }
}

// add vocab logic

void add_word(int *fd, char *buffer, int size)
{

    ssize_t bytes_read = read(*fd, buffer, size - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        vocab_entry ve;
        memset(&ve, 0, sizeof(vocab_entry));

        char *destinations[] = {ve.language, ve.german, ve.translation, ve.type, ve.gender};
        size_t sizes[] = {sizeof(ve.language), sizeof(ve.german), sizeof(ve.translation), sizeof(ve.type), sizeof(ve.gender)};
        int num_fields = sizeof(destinations) / sizeof(destinations[0]);

        char *token = strtok(buffer, "|");

        for (int i = 0; i < num_fields; i++)
        {
            if (token == NULL)
                return;
            strncpy(destinations[i], token, sizes[i]);
            destinations[i][sizes[i] - 1] = '\0';
            token = strtok(NULL, "|");
        }
        insert_vocab(&ve);
    }
    else if (bytes_read == 0)
    {
        close(*fd);
        *fd = open(PIPE_PATH, O_RDWR);
    }
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

            while (keep_running)
            {
                char buffer[256];
                int ret = poll(&pfd, 1, 5000);
                if (ret == 0)
                {
                    // * Timer for polling is over -> prompt for next vocab
                    check_and_prompt_vocab();
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
                    add_word(&fd, buffer, sizeof(buffer));
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