#pragma once
#include "vocab_entry.h"

#define BASE_EASE_FACTOR 2.5

int init_sm2_stats(vocab_entry *ve);
int calculate_sm2(vocab_entry *ve, int lvl);