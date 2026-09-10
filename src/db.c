#include "db.h"
#include "paths.h"
#include "vocab_entry.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>

int get_due_vocab(vocab_entry **found_entries, size_t *count)
{
    // vocab found returns  0, not found 1, error -1
    time_t now = time(NULL);
    FILE *fvoc = fopen(get_storage_filepath(), "rb");
    int vocab_found = 0;
    size_t capacity = 10;
    *count = 0;
    *found_entries = malloc(capacity * sizeof(vocab_entry));
    if (*found_entries == NULL)
    {
        // * alloc error
        return -1;
    }
    if (fvoc == NULL)
    {
        // TODO Logging
        return -1;
    }
    vocab_entry ve;
    while (fread(&ve, sizeof(vocab_entry), 1, fvoc))
    {
        if (ve.next_due <= now)
        {
            (*found_entries)[*count] = ve;
            vocab_found = 1;
            (*count)++;
        }
        if (*count >= capacity)
        {
            capacity *= 2;
            vocab_entry *tmp = realloc(*found_entries, capacity * sizeof(vocab_entry));
            if (tmp == NULL)
            {
                free(*found_entries);
                return -1;
            }
            *found_entries = tmp;
        }
    }
    fclose(fvoc);
    return !vocab_found;
}

int get_all_vocabs(vocab_entry **found_entries, size_t *count)
{
    // vocab found returns  0, not found 1, error -1
    time_t now = time(NULL);
    FILE *fvoc = fopen(get_storage_filepath(), "rb");
    int vocabs = 0;
    size_t capacity = 10;
    *count = 0;
    *found_entries = malloc(capacity * sizeof(vocab_entry));
    if (*found_entries == NULL)
    {
        // * alloc error
        return -1;
    }
    if (fvoc == NULL)
    {
        // TODO Logging
        free(*found_entries);
        return -1;
    }
    vocab_entry ve;
    while (fread(&ve, sizeof(vocab_entry), 1, fvoc))
    {
        (*found_entries)[*count] = ve;
        vocabs = 1;
        (*count)++;
        if (*count >= capacity)
        {
            capacity *= 2;
            vocab_entry *tmp = realloc(*found_entries, capacity * sizeof(vocab_entry));
            if (tmp == NULL)
            {
                free(*found_entries);
                return -1;
            }
            *found_entries = tmp;
        }
    }
    fclose(fvoc);
    return !vocabs;
}

int update_vocab(vocab_entry *entry)
{
    FILE *f = fopen(get_storage_filepath(), "r+b");

    if (f == NULL)
    {
        // TODO logging
        return 1;
    }
    flock(fileno(f), LOCK_EX);

    vocab_entry tmp_ve;
    while (fread(&tmp_ve, sizeof(vocab_entry), 1, f))
    {
        if (tmp_ve.uid != entry->uid)
            continue;

        fseek(f, -sizeof(vocab_entry), SEEK_CUR);
        fwrite(entry, sizeof(vocab_entry), 1, f);
        break;
    }
    flock(fileno(f), LOCK_UN);
    fclose(f);
    return 0;
}

int insert_vocab(vocab_entry *entry)
{
    FILE *f = fopen(get_storage_filepath(), "a+b");
    if (f == NULL)
    {
        // TODO Logging
        return 1;
    }
    flock(fileno(f), LOCK_EX);

    vocab_entry ve;
    if (fseek(f, -sizeof(vocab_entry), SEEK_END) >= 0)
    {
        fread(&ve, sizeof(vocab_entry), 1, f);
        entry->uid = ve.uid + 1;
    }
    else
    {
        rewind(f);
        entry->uid = 1;
    }
    fwrite(entry, sizeof(vocab_entry), 1, f);
    flock(fileno(f), LOCK_UN);
    fclose(f);
    return 0;
}