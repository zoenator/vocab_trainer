#include <time.h>
#define _XOPEN_SOURCE 700

#include "paths.h"
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
void prompt_vocab()
{
    vocab_entry cur_ve;

    time_t now = time(NULL);
    FILE *fvoc = fopen(get_storage_filepath(), "r+b");
    if (fvoc != NULL)
    {
        while (fread(&cur_ve, sizeof(vocab_entry), 1, fvoc))
        {
            if (cur_ve.next_due <= now)
            {
                char cmd[512];
                // TODO Implement logic here to choose between first to second language or second to first language

                // * create command string for notification
                snprintf(cmd,
                         sizeof(cmd),
                         "notify-send --action='default=Antworten' -t 10000 'Vocab Trainer' 'Translate: %s to %s'",
                         cur_ve.german,
                         cur_ve.language);

                FILE *fp = popen(cmd, "r");
                char action[64] = {0};
                fgets(action, sizeof(action), fp);
                pclose(fp);
                if (strncmp(action, "default", strlen("default")) == 0)
                {
                    // * create command string for zenity prompt
                    snprintf(cmd,
                             sizeof(cmd),
                             "zenity --entry --title='Vocab Trainer' --text='Was heißt %s auf %s?'",
                             cur_ve.german,
                             cur_ve.language);
                    FILE *fp = popen(cmd, "r");
                    char answer[128];
                    fgets(answer, sizeof(answer), fp);
                    pclose(fp);
                    answer[strcspn(answer, "\r\n")] = '\0';
                    if (strcmp(cur_ve.translation, answer) == 0)
                    {
                        // * correct answer
                        cur_ve.difficulty < 4 ? cur_ve.difficulty++ : cur_ve.difficulty;
                        cur_ve.next_due = now + cur_ve.difficulty * 86400;
                    }
                    else
                    {
                        // * incorrect answer
                        /*
                            ! rn correct answer has no margin its either right or wrong, implement system that can evaluate
                            ! how close u were to the real solution
                        */
                        cur_ve.difficulty > 0 ? cur_ve.difficulty-- : cur_ve.difficulty;
                        cur_ve.next_due = now + 60;
                    }
                }
                else
                {
                    cur_ve.next_due = now + 300;
                }
                cur_ve.last_occurence = now;
                fseek(fvoc, -sizeof(vocab_entry), SEEK_CUR);
                fwrite(&cur_ve, sizeof(vocab_entry), 1, fvoc);
                break;
            }
        }
        fclose(fvoc);
    }
}

// add vocab logic

void add_word(int *fd, char *buffer, int size)
{

    ssize_t bytes_read = read(*fd, buffer, size - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        FILE *f = fopen(get_storage_filepath(), "ab");
        if (f)
        {
            vocab_entry ve;
            memset(&ve, 0, sizeof(vocab_entry));
            char *token = strtok(buffer, "|");

            strcpy(ve.language, token);
            token = strtok(NULL, "|");

            strcpy(ve.german, token);
            token = strtok(NULL, "|");

            strcpy(ve.translation, token);
            token = strtok(NULL, "|");

            strcpy(ve.type, token);
            token = strtok(NULL, "|");

            strcpy(ve.gender, token);
            token = strtok(NULL, "|");

            ve.difficulty = 0;
            ve.last_occurence = time(NULL);
            ve.next_due = time(NULL);

            fwrite(&ve, 1, sizeof(vocab_entry), f);
            fclose(f);
        }
    }
    else if (bytes_read == 0)
    {
        close(*fd);
        *fd = open("/tmp/vocab_pipe", O_RDWR);
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
            mkfifo("/tmp/vocab_pipe", 0666);

            // * open pipe in readonly
            int fd = open("/tmp/vocab_pipe", O_RDWR);

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
                    prompt_vocab();
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
            unlink("/tmp/vocab_pipe");
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
    setup_signal_handling();
    daemonize();

    return 0;
}