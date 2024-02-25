#include <iostream>

#define width_of_chunk 16
#define height_of_chunk 256
#define volume_of_chunk width_of_chunk*height_of_chunk

struct chunk{
    int chunk_blocks[volume_of_chunk];
    int changes[100];


    //chunk() :{}
};

int main(){
    chunk *loadded_chunk[10];
    loadded_chunk[0] = new(chunk);
    //std::cout << loadded_chunk[0].x << std::endl;
    //std::cout << loadded_chunk[0].y << std::endl;
}
