#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int parse_lueckentext(const char *input, char *display_text, char *solution, size_t display_size, size_t solution_size)
{
    char *gap_start = strchr(input, '{');
    char *gap_end = strchr(input, '}');

    if (gap_end == NULL || gap_start == NULL || gap_start >= gap_end)
    {
        strncpy(display_text, input, display_size - 1);
        display_text[display_size - 1] = '\0';
        return 1;
    }

    size_t word_len = gap_end - gap_start - 1;
    if (word_len >= solution_size)
        word_len = solution_size - 1;
    strncpy(solution, gap_start + 1, word_len);
    solution[word_len] = '\0';

    snprintf(display_text, display_size, "%.*s ___ %s", (int)(gap_start - input), input, gap_end + 1);
    return 0;
}