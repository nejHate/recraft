#include <iostream>
#include <array>
#include <vector>

struct struct1{
    uint8_t x;
};

struct struct2{
    uint16_t x;
};

struct struct3{
    uint32_t x;
};

struct struct4{
    uint64_t x;
};

int main(){
    void *blockptr[100];
    std::vector<void> blockarray;
    int x = 0;
    for(int i = 0; i < 100; i++){
        x = i%4;
        if(x == 0){
            blockptr[i] = new struct1;
        }
        if(x == 1){
            blockptr[i] = new struct2;
        }
        if(x == 2){
            blockptr[i] = new struct3;
        }
        if(x == 3){
            blockptr[i] = new struct4;
        }
    }
    for(int i = 0; i < 100; i++){
        std::cout << blockptr[i] << " " << std::endl;
    }
}