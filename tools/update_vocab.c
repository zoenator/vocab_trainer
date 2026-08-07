#include "vocab_entry.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    // convert argument to int
    int index = strtol(argv[1], NULL, 10);

    // open vocab file in binary reading mode
    FILE *f = fopen("/tmp/vocab.bin", "r+b");
    // check if file opened
    if (f == NULL)
        return 1;

    // seek given vocab entry
    fseek(f, index * sizeof(vocab_entry), SEEK_SET);

    // read found vocab entry into buffer
    vocab_entry ve;
    fread(&ve, sizeof(vocab_entry), 1, f);

    ve.mistakes++;

    // jump back for write bakc (fread moves pointer behind actual block)
    fseek(f, index * sizeof(vocab_entry), SEEK_SET);
    fwrite(&ve, sizeof(vocab_entry), 1, f);

    fclose(f);
    return 0;
}