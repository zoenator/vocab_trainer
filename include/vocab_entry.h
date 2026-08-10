#pragma once
#include <stdint.h>
#include <time.h>

typedef struct vocab_entry
{
        uint32_t uid;
        int is_deleted;
        char language[16];

        char german[256];
        char translation[256];
        char example_sentence[256];

        time_t last_occurence;
        time_t next_due;

        unsigned int mistakes;

        char type[8];   // TODO implement own datatype for type
        char gender[4]; // TODO implement own datatype for gender

        int difficulty; //* calculates from average mistakes on the word

        // TODO implement audiofile

} vocab_entry;
