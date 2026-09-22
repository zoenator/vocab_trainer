#include "db.h"
#include "paths.h"
#include "sm2.h"
#include "utils.h"
#include "vocab_entry.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

int cmd_add(int argc, char **argv)
{
    // TODO This needs a more dynamic redesign (Here you cant type in gender without giving a tpye too which is kinda meh)
    // TODO Needs error management !USER INPUT!
    if (argc < 5)
    {
        printf("Usage: Vocab_CLI add <lang> <front_text> <back_text> [entry_type] [tags]");
        return EX_USAGE;
    }

    char *entry_type = (argc >= 6) ? argv[5] : "0";
    const char *tags = (argc >= 7) ? argv[6] : "0";

    vocab_entry ve;
    memset(&ve, 0, sizeof(vocab_entry));
    init_sm2_stats(&ve);

    strncpy(ve.language, argv[2], sizeof(ve.language));
    ve.language[sizeof(ve.language) - 1] = '\0';

    strncpy(ve.front_text, argv[3], sizeof(ve.front_text));
    ve.front_text[sizeof(ve.front_text) - 1] = '\0';

    strncpy(ve.back_text, argv[4], sizeof(ve.back_text));
    ve.back_text[sizeof(ve.back_text) - 1] = '\0';

    if (argc >= 6)
    {
        char *endptr;
        ve.entry_type = strtol(entry_type, &endptr, 10);
    }
    else
    {
        ve.entry_type = -1;
    }

    if (argc >= 7)
    {
        strncpy(ve.tags, tags, sizeof(ve.tags));
        ve.tags[sizeof(ve.tags) - 1] = '\0';
    }
    else
    {
        ve.tags[0] = '\0';
    }

    time_t now = time(NULL);
    ve.creation_date = now;

    int error = insert_vocab(&ve);
    if (error)
    {
        PRINT_USR_ERR("Inserting vocab didn't work");
        return EX_OSERR;
    }
    return 0;
}

int cmd_stats(int argc, char **argv)
{

    // * get vocabs and calculate stats
    vocab_entry *entries;
    int exit_status = 0;
    size_t count = 0;

    exit_status = get_all_vocabs(&entries, &count);
    if (exit_status == -1)
    {
        PRINT_USR_ERR("Error getting vocabs");
        return EX_OSERR;
    }

    // stat variables
    // TODO expand stats (e.G solved exercises/day ... )
    int total_words = 0;
    int *different_languages = calloc(2, sizeof(int));
    char **languages = calloc(2, sizeof(char *));
    int capacity = 2;
    if (languages == NULL)
    {
        PRINT_ERR("Allocation error");
        return EX_OSERR;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (entries[i].is_deleted)
            continue;

        total_words++;

        unsigned int exists = 0;
        unsigned int language_count = 0;
        for (size_t j = 0; languages[j] != NULL; j++)
        {
            if (strcmp(entries[i].language, languages[j]) == 0)
            {
                exists = 1;
                break;
            }
            language_count++;
        }
        if (exists)
        {
            different_languages[language_count]++;
            continue;
        }
        if (language_count >= capacity)
        {
            void *tmp = realloc(different_languages, sizeof(int) * capacity * 2);
            if (tmp == NULL)
            {
                printf("realloc error\n");
                exit_status = EX_OSERR;
                goto EXIT;
            }
            different_languages = tmp;
            memset(different_languages + capacity, 0, sizeof(int) * capacity);
            tmp = realloc(languages, sizeof(char *) * capacity * 2);
            if (tmp == NULL)
            {
                PRINT_ERR("Allocation error");
                exit_status = EX_OSERR;
                goto EXIT;
            }
            languages = tmp;
            memset(languages + capacity, 0, sizeof(char *) * capacity);
            capacity *= 2;
        }

        languages[language_count] = strdup(entries[i].language);
        different_languages[language_count]++;
    }

    printf("Total words: %i\n", total_words);

EXIT:
    for (size_t i = 0; languages[i] != NULL; i++)
    {
        printf("%s words: %d\n", languages[i], different_languages[i]);
        free(languages[i]);
    }

    free(entries);
    free(languages);
    free(different_languages);

    return exit_status;
}

int cmd_mode(int argc, char **argv)
{
    if (argc <= 2 || (strcmp(argv[2], "PASSIVE") != 0) && (strcmp(argv[2], "ACTIVE") != 0))
    {
        printf("Usage: VocabTrainer mode <mode> \n Modes: \n'PASSIVE'\n'ACTIVE'\n");
        return EX_USAGE;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "MODE|%s", argv[2]);
    int fd = open(PIPE_PATH, O_WRONLY);
    if (fd == -1)
    {
        PRINT_ERR("Error: Pipe not found");
        return EX_UNAVAILABLE;
    }
    int bytes_read = write(fd, msg, strlen(msg));

    if (bytes_read == -1)
    {
        PRINT_ERR("Failed to write to pipe");
        if (close(fd) == -1)
        {
            PRINT_ERR("Failed to close file descriptor");
        }
        return EX_UNAVAILABLE;
    }

    if (close(fd) == -1)
    {
        PRINT_ERR("Failed to close file descriptor");
    }
    return 0;
}

int cmd_list(int argc, char **argv)
{
    // TODO argc/argv might be used for filtering in the future
    vocab_entry *entries;
    size_t count = 0;
    int exit_status = 0;
    exit_status = get_all_vocabs(&entries, &count);
    if (exit_status == -1)
    {
        free(entries);
        PRINT_ERR("Getting vocabs failed");
        exit_status = EX_OSERR;
        return exit_status;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (entries[i].is_deleted)
            continue;

        printf("[%d]: %s -> %s\n", entries[i].uid, entries[i].front_text, entries[i].back_text);
    }

    return exit_status;
}

int cmd_delete(int argc, char **argv)
{
    // TODO Implement deleting of several uids in one call
    if (argc < 3 || argv == NULL)
        return 1;
    char *endptr;
    long uid = strtol(argv[2], &endptr, 10);
    if (argv[2] == endptr)
    {
        PRINT_USR_ERR("No valid number!");
        return EX_DATAERR;
    }

    vocab_entry *entries;
    size_t count = 0;
    int exit_status = 0;
    exit_status = get_all_vocabs(&entries, &count);
    if (exit_status == -1)
    {
        free(entries);
        PRINT_ERR("Getting vocabs failed");
        exit_status = EX_OSERR;
        return exit_status;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (entries[i].uid != uid)
            continue;

        entries[i].is_deleted = 1;
        int error = update_vocab(&(entries[i]));
        if (error != 0)
        {
            PRINT_ERR("Updateing vocab failed");
            exit_status = EX_OSERR;
        }
        else
        {
            printf("Entry %u deleted. (%s -> %s)\n", entries[i].uid, entries[i].front_text, entries[i].back_text);
        }
        break;
    }

    return exit_status;
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("Usage: Vocab_cli <command> [args]\nCommands: add, stats\n");
        return 1;
    }

    if (strcmp(argv[1], "add") == 0)
    {
        return cmd_add(argc, argv);
    }
    else if (strcmp(argv[1], "stats") == 0)
    {
        return cmd_stats(argc, argv);
    }
    else if (strcmp(argv[1], "mode") == 0)
    {
        return cmd_mode(argc, argv);
    }
    else if (strcmp(argv[1], "list") == 0)
    {
        return cmd_list(argc, argv);
    }
    else if (strcmp(argv[1], "delete") == 0)
    {
        return cmd_delete(argc, argv);
    }
    else
    {
        printf("Unknown command: %s\n", argv[1]);
        return EX_USAGE;
    }

    return 0;
}