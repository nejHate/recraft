#include <iostream>
#include <unordered_map>
#include <random>


#define width_of_chunk 16
#define height_of_chunk 256
#define volume_of_chunk width_of_chunk*height_of_chunk

struct block{
    int block;
};
struct chunk{
    int chunk_x;
    int chunk_y;
    chunk *up;
    int up_ptr;
    chunk *right;
    int right_ptr;
    chunk *down;
    int down_ptr;
    chunk *left;
    int left_ptr;
    block chunk_blocks[volume_of_chunk];
    int changes[100];

    chunk(){}

    chunk(int x_chunk, int y_chunk, chunk* up, chunk* right, chunk* down, chunk* left) :
        chunk_x(chunk_x),
        chunk_y(chunk_y),
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
        //std::cout << "allocate chunk" << std::endl;
    }
};
class Chunk_Manager{
    private:
        std::unordered_map<int, chunk*> mapped_chunks;
    public:
        void add_chunk(int x, int z, chunk* ptr){
            u_int64_t suma = static_cast<int64_t>(x) << 32 | z;
            mapped_chunks[suma] = ptr;
        }
        chunk* find_chunk(int x, int z){
            u_int64_t suma = static_cast<int64_t>(x) << 32 | z;
            auto it = mapped_chunks.find(suma);
            if (it != mapped_chunks.end()){
                return it->second;
            }
            else{
                return nullptr;
            }
        }
        void delete_chunk(int x, int z){
            u_int64_t suma = static_cast<int64_t>(x) << 32 | z;
            mapped_chunks.erase(suma);
        }
};

int main(){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    std::cout << dis(gen) << std::endl;
    uint32_t lol1 = -1;
    uint64_t lol2 = (static_cast<u_int64_t>(lol1) << 32) | 1;  // 18446744069414584321
    std::cout << lol2 << std::endl;

    Chunk_Manager hashed_chunks;

    chunk *loadded_chunks[10];
    for(int i = 0; i < 10; i++){
        std::cout << hashed_chunks.find_chunk(i+1, 0) << " " << hashed_chunks.find_chunk(i, 0+1) << " " \
                  << hashed_chunks.find_chunk(i-1, 0) << " " << hashed_chunks.find_chunk(i, 0-1) << std::endl;
        loadded_chunks[i] = new chunk(i, 0, hashed_chunks.find_chunk(i+1, 0),\
                                                hashed_chunks.find_chunk(i, 0+1),\
                                                hashed_chunks.find_chunk(i-1, 0),\
                                                hashed_chunks.find_chunk(i, 0-1));
        hashed_chunks.add_chunk(i, 0, loadded_chunks[i]);
    }
    for (int i = 0; i < 10; i++){
        //hashed_chunks.delete_chunk(loadded_chunks[i]->chunk_x, loadded_chunks[i]->chunk_y);
        delete loadded_chunks[i];
    }


    for (int i = 0; i < 10; i++){
        loadded_chunks[i] = new chunk(0, 0, nullptr, nullptr, nullptr, nullptr);
    }
    for (int i = 0; i < 10; i++){
        delete loadded_chunks[i];
    }
    //std::cout << loadded_chunk[0].x << std::endl;
    //std::cout << loadded_chunk[0].y << std::endl;
    std::cout << "END OF PROGRAM #4" << std::endl;
}

//g++ -fwhole-program -O3 -g -fsanitize=address -fsanitize=leak main.cpp -o main && ./main && g++ ./main.cpp -o main && valgrind ./main