#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int parse_lueckentext(const char *input, char *display_text, char *solution, size_t display_size, size_t solution_size)
{
    char *gap_start = strchr(input, '{');
    char *gap_end = strchr(input, '}');

    if (gap_end == NULL || gap_start == NULL)
    {
        strncpy(display_text, input, strlen(input));
        display_text[strlen(input)] = '\0';
        return 1;
    }

    size_t word_len = gap_end - gap_start - 1;
    if (word_len >= solution_size)
        word_len = solution_size - 1;
    strncpy(solution, gap_start + 1, word_len);
    solution[word_len] = '\0';

    char display_start[256] = {0};
    char display_gap[] = "___";

    strncpy(display_start, input, gap_start - input);
    display_start[gap_start - input] = '\0';

    snprintf(display_text, display_size, "%s %s %s", display_start, display_gap, gap_end + 1);
    return 0;
}