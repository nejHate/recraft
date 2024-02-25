#include <iostream>

#define width_of_chunk 16
#define height_of_chunk 256
#define volume_of_chunk width_of_chunk*height_of_chunk

struct chunk{
    chunk *up;
    int up_ptr;
    chunk *right;
    int right_ptr;
    chunk *down;
    int down_ptr;
    chunk *left;
    int left_ptr;
    int chunk_blocks[volume_of_chunk];
    int changes[100];

    chunk(){}

    chunk(chunk* up, chunk* right, chunk* down, chunk* left) :
        up(up),
        right(right),
        down(down),
        left(left),
        up_ptr(0),
        right_ptr(0),
        down_ptr(0),
        left_ptr(0)
    {
        if(up != nullptr){
            up->down = this;
        }
        if(right != nullptr){
            right->left = this;
        }
        if(down != nullptr){
            down->up = this;
        }
        if(left != nullptr){
            left->right = this;
        }
    }
};

int main(){
    chunk *loadded_chunk[10];
    for (int i = 0; i < 10; i++){
        loadded_chunk[i] = new chunk(nullptr, nullptr, nullptr, nullptr);
    }
    for (int i = 0; i < 10; i++){
        delete loadded_chunk[i];
    }
    //std::cout << loadded_chunk[0].x << std::endl;
    //std::cout << loadded_chunk[0].y << std::endl;
    std::cout << "END OF PROGRAM" << std::endl;
}
