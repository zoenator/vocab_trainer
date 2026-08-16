#include "eval.h"
#include "vocab_entry.h"
#include <stdint.h>
#include <string.h>

#define MIN(a, b, c) (((a) < (b)) ? ((a) < (c)) ? (a) : (c) : ((b) < (c)) ? (b) : (c))

uint16_t apply_levenshtein(const char *answer, const char *solution)
{
    // *  calculates and returns levenshtein distance
    int dimA = strlen(answer) + 1;
    int dimB = strlen(solution) + 1;

    uint16_t v0[STD_ARRAY_SIZE + 1] = {0};
    uint16_t v1[STD_ARRAY_SIZE + 1] = {0};

    for (int i = 0; i < dimB; i++)
    {
        v0[i] = i;
    }

    uint16_t *prev = v0;
    uint16_t *curr = v1;

    for (int i = 0; i < dimA - 1; i++)
    {
        curr[0] = i + 1;
        for (int j = 0; j < dimB - 1; j++)
        {
            int deletion = prev[j + 1];
            int insertion = curr[j] + 1;
            int substitution = prev[j];
            if (answer[i] != solution[j])
                substitution += 1;
            curr[j + 1] = MIN(deletion, insertion, substitution);
        }
        uint16_t *tmp = NULL;
        tmp = prev;
        prev = curr;
        curr = tmp;
    }

    return *(prev + dimB - 1);
}

int calculate_level(int levenshtein_distance, unsigned int time_taken_seconds, uint16_t word_length)
{
    int level = 5;
    int error_quota = levenshtein_distance * 100 / word_length;
    int time_penalty = time_taken_seconds > 5;
    level = level - (error_quota / 20) - time_penalty;
    return (level >= 0 ? level : 0);
}