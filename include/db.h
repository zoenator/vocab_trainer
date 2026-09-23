#pragma once
#include "vocab_entry.h"

int get_vocab_by_id(vocab_entry *entry, unsigned long id);
int get_due_vocab(vocab_entry **found_entries, size_t *count);
int get_all_vocabs(vocab_entry **found_entries, size_t *count);
int update_vocab(vocab_entry *entry);
int insert_vocab(vocab_entry *entry);