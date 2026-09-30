#pragma once
#include <stddef.h>

int fetch_translation(const char *source_word, const char *source_lang, const char *target_lang, char *result_buffer, size_t buffer_size);