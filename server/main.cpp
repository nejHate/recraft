#include <iostream>
#include <unordered_map>
#include <random>
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>
#include <filesystem>


#define width_of_chunk 16
#define height_of_chunk 256
#define volume_of_chunk width_of_chunk*width_of_chunk*height_of_chunk
#define max_saved_chunk_size volume_of_chunk*sizeof(block)

struct block{
    u_int32_t block;
    //int block2;
};
struct chunk{
    int chunk_x;
    int chunk_z;
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

    chunk(int chunk_x, int chunk_z, chunk* up, chunk* right, chunk* down, chunk* left) :
        chunk_x(chunk_x),
        chunk_z(chunk_z),
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
        std::cout << "creation of chunk: " << chunk_x << " " << chunk_z << std::endl;
    }
};
class Chunk_Manager{
    private:
        std::unordered_map<uint64_t, chunk*> mapped_chunks;
    public:
        void add_chunk(int x, int z, chunk* ptr){
            u_int64_t suma = static_cast<u_int64_t>(x) << 32 | z;
            std::cout << "adding hashed_chunk: " << x << " " << z << std::endl;
            mapped_chunks[suma] = ptr;
        }
        chunk* find_chunk(int x, int z){
            u_int64_t suma = static_cast<u_int64_t>(x) << 32 | z;
            auto it = mapped_chunks.find(suma);
            if (it != mapped_chunks.end()){
                return it->second;
            }
            else{
                return nullptr;
            }
        }
        void delete_chunk(int x, int z){
            u_int64_t suma = static_cast<u_int64_t>(x) << 32 | z;
            std::cout << "deleting hashed_chunk: " << x << " " << z << std::endl;
            mapped_chunks.erase(suma);
        }
};


bool file_exists(const std::string& filename){
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

chunk* create_chunk(int x, int z, Chunk_Manager &hashed_chunks){
    chunk *new_chunk = new chunk(x, z, hashed_chunks.find_chunk(x+1, z),  \
                                       hashed_chunks.find_chunk(x,   z+1),\
                                       hashed_chunks.find_chunk(x-1, z),  \
                                       hashed_chunks.find_chunk(x,   z-1));
    hashed_chunks.add_chunk(x, z, new_chunk);
    return(new_chunk);
}

chunk* load_chunk(std::string& base, int x, int z, Chunk_Manager &hashed_chunks, bool end){
    static char *loaded_file = new(std::nothrow) char[max_saved_chunk_size];
    if (loaded_file == nullptr){
        std::cout << "ERROR TO ALLOCATE ARRAY FOR LOADING FILE" << " in file: " << __FILE__ << ":" << __LINE__ << std::endl;
    }
    if (end == 1){
        delete[] loaded_file;
        return nullptr;
    }

    std::string filename = base + std::to_string(x) + "." + std::to_string(z) + ".chunk";
    std::cout << "searching for file: " << filename << std::endl;
    if (file_exists(filename)) {
        std::cout << "file exists: " << filename << std::endl;
        std::ifstream file(filename);
        if (!file.is_open()){
            std::cerr << "ERROR WHEN OPENING FILE: " << filename << " in file: " << __FILE__ << ":" << __LINE__ << std::endl;
        }
        std::error_code ec{};
        auto file_size = std::filesystem::file_size(filename, ec);
        if (ec) {
            std::cerr << "ERROR WHEN GETING LENGHT OF FILE: " << filename << " in file: " << __FILE__ << ":" << __LINE__ << " " << ec.message() << std::endl;
        }
        file.read(loaded_file, file_size);
        loaded_file[file_size] = '\0';  // DELETE don't needed when not printing
        std::cout << "size of file: " << file_size << std::endl;

        std::cout << "file content: " << loaded_file << std::endl;

        chunk *new_chunk = create_chunk(x, z, hashed_chunks);
        file.close();
        return new_chunk;

    }
    else {
        std::cout << "file not exists: " << filename << std::endl;
        chunk *new_chunk = create_chunk(x, z, hashed_chunks);
        return new_chunk;
    }
}



int main(){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, INT32_MAX);
    std::cout << "first random number: " << dis(gen) << std::endl;

    Chunk_Manager hashed_chunks;

    std::string base = "./data/";
    chunk *test;
    int test_size = 2;
    for(int i = 0; i < test_size; i++){
        for(int y = 0; y < test_size; y++){
            load_chunk(base, i, y, hashed_chunks, 0);
        }
    }
    exit(0);
    for(int i = 0; i < test_size; i++){
        for(int y = 0; y < test_size; y++){
            delete (hashed_chunks.find_chunk(i, y));
            hashed_chunks.delete_chunk(i, y);
            std::cout << "deleting chunk: " << i << " " << y << std::endl;
        }
    }
    if(load_chunk(base, 1, 1, hashed_chunks, 1)){
        std::cout << "ERROR WHEN CLEANING FUNCTION load_chunk " << " in file: " << __FILE__ << ":" << __LINE__ << std::endl;
    }
    std::cout << sizeof(chunk) << " " << sizeof(chunk) * test_size / 1000000 << std::endl;
    std::cout << "END OF PROGRAM #5" << std::endl << std::endl;
}

//clear && g++ -fwhole-program -O3 -g -fsanitize=address -fsanitize=leak main.cpp -o main && ./main && g++ ./main.cpp -g -o main && valgrind -s --leak-check=full ./main
//clear && g++ -fwhole-program -O3 -g -fsanitize=address -fsanitize=leak main.cpp -o main && ./main && g++ ./main.cpp -g -o main && valgrind -s --leak-check=full --show-leak-kinds=all ./main

//g++ ./main.cpp -O3 -fwhole-program -mavx2
//valgrind -v -s --trace-children=yes --track-fds=all --time-stamp=yes --log-file=out.txt --error-limit=no --read-inline-info=yes --read-var-info=yes --leak-check=full --leak-resolution=high --show-leak-kinds=all --leak-check-heuristics=all --show-reachable=yes --show-possibly-lost=yes --track-origins=yes --expensive-definedness-checks=yes --malloc-fill=ee --free-fill=dd ./main


/*
while True:

    a = [["a", ".-"], ["b", ".-"]]
    task = random.choice(list)
    print(task)
    if input() == task:
        print("correct!")
    else:
        print("incorrect!")*/
