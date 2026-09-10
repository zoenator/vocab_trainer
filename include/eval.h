#pragma once
#include "vocab_entry.h"

#include <stdint.h>

uint16_t apply_levenshtein(const char *answer, const char *solution);
int calculate_level(int levenshtein_distance, unsigned int time_taken_seconds, uint16_t word_length);
vocab_entry *get_most_urgent_vocab(vocab_entry *entries, size_t count);