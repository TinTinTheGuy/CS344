#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
#define ALPHABET_LENGTH 27

int main(int argc, char* argv[]) {
    int i, index, keyLength;
    
    if (argc < 2) {
        printf("Please provide a key length as an argument.\n");
        return 1;
    }
    keyLength = atoi(argv[1]);
    
    if (keyLength <= 0) {
        printf("Key length must be a positive integer.\n");
        return 1;
    }
    
    srand(time(NULL));
    char key[keyLength + 2]; // Extra space for newline and null terminator
    for (i = 0; i < keyLength; i++) {
        index = rand() % ALPHABET_LENGTH;
        key[i] = ALPHABET[index];
    }
    
    key[keyLength] = '\n';
    key[keyLength + 1] = '\0';

    printf("%s", key);
    return 0;
}
