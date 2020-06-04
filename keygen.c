#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char* argv[]){
    const char letters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    srand(time(NULL));
    if(argc != 2){fprintf(stderr,"USAGE: %s keylength\n", argv[0]); exit(0); }
    int length = atoi(argv[1]);
    char* key;
    key = (char*)malloc((sizeof(char) * length)+1);
    int i =0;
    for(i = 0; i < length; i++){
        int randint = rand() % 27;
        key[i]=letters[randint];
    }
    key[i] = '\0';
    printf("%s\n", key);
    free(key); //freaky
}

