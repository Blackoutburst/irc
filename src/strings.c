#include "strings.h"

I32 stringCountWord(const I8* string, const I8 delimiter) {
    I32 count = 1;
    
    for (U32 i = 0; string[i]; i++) {
        if (string[i] == delimiter) count++;
    }

    return count;
}

I8** stringSplit(const I8* string, const I8 delimiter) {
    U8 wordCount = stringCountWord(string, delimiter);
    U16 start = 0;
    I8** output = malloc(sizeof(I8*) * (wordCount + 1));

    output[wordCount] = NULL;

    for (U16 i = 0; i < wordCount; i++) {
        U16 offset = 0;
        while (string[start + offset] != delimiter && string[start + offset] != '\0') offset++;
        offset += 1;

        output[i] = malloc(offset);

        for (U16 j = 0; j < offset; j++) {
            output[i][j] = string[start + j];
        }
        output[i][offset -1] = '\0';
        start += offset;
    }

    return output;
}

void stringCleanArray(I8** string_array) {
    for (U16 i = 0; string_array[i]; i++) {
        free(string_array[i]);
    }
    
    free(string_array);
}

void stringClear(I8* string) {
    U32 length = strlen(string);
    
    for (U32 i = 0; i < length; i++) string[i] = '\0';
}

