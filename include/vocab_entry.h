#pragma once
#include <stdint.h>
#include <time.h>

#define SEC_PER_DAY 86400
#define PENALTY_DELAY 60
#define USER_LANG "DE"
#define STD_ARRAY_SIZE 256
#define TAG_SIZE 128

typedef struct vocab_entry
{
        // * meta data
        uint32_t uid;
        int is_deleted;
        time_t creation_date;

        // * classification
        char language[16];
        uint8_t entry_type;
        char tags[TAG_SIZE];

        // * content
        char front_text[STD_ARRAY_SIZE];
        char back_text[STD_ARRAY_SIZE];
        char example_sentence[STD_ARRAY_SIZE];

        // * SM2 & Stats
        unsigned int mistakes;
        unsigned int correct_answers;
        time_t last_occurence;
        time_t next_due;
        uint8_t repetitions;
        float ease_factor;
        int interval;

        // * media
        char audio_file[128];
        char image_file[128];

} vocab_entry;
