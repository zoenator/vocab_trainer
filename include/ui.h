#pragma once
#include <stdio.h>
#include <string.h>

int ui_play_audio(const char *lang_code, const char *text);
int ui_display_vocab_passive(char *front_text, char *back_text);
int ui_prompt_translation(char *origin_word, char *language, char *answer_buffer, size_t answer_size, int show_audio);
int ui_show_feedback(const char *correct_word, int distance, int level);