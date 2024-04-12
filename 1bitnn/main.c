#include <stdlib.h>
#include <stdio.h>

int main(){
    __uint64_t x = -1;
    __uint64_t y = __builtin_popcountll(x);
    __int64_t z[28*28];
    printf("%ld\n", y);
    printf("%ld\n", 8000000000/(1920*1080)/60);
}