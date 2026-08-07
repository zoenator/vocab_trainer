#include "paths.h"
#include "vocab_entry.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    // convert argument to int
    if (argc < 2)
        return 1;
    int index = strtol(argv[1], NULL, 10);

    // open vocab file in binary reading mode
    FILE *f = fopen(get_storage_filepath(), "rb");
    // check if file opened
    if (f == NULL)
        return 1;

    // seek given vocab entry

    fseek(f, index * sizeof(vocab_entry), SEEK_SET);

    // read found vocab entry into buffer
    vocab_entry ve;
    fread(&ve, sizeof(vocab_entry), 1, f);

    printf("%s\n", ve.german);
    printf("%s\n", ve.translation);
    printf("%d\n", ve.mistakes);

    fclose(f);
    return 0;
}