#pragma once
#include <stdio.h>
#include <string.h>

void ui_display_vocab_passive(char *front_text, char *back_text);
void ui_prompt_translation(char *origin_word, char *language, char *answer_buffer, size_t answer_size);
void ui_show_feedback(const char *correct_word, int distance, int level);