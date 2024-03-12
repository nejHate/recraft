#include <iostream>
#include <unordered_map>
#include <random>
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>
#include <filesystem>
#include <cstring>
#include <bitset>
#include <thread>
#include <atomic>


#define width_of_chunk 16
#define height_of_chunk 256
#define size_of_chunk width_of_chunk*width_of_chunk*height_of_chunk
#define max_saved_chunk_size size_of_chunk*sizeof(block)

struct block{
    std::bitset<32> block;
    //void block;
    //int block2;
};
struct chunk{
    int chunk_x;
    int chunk_z;
    chunk *chunk_z_minus;
    int up_ptr;
    chunk *chunk_x_plus;
    int right_ptr;
    chunk *chunk_z_plus;
    int down_ptr;
    chunk *chunk_x_minus;
    int left_ptr;
    block chunk_blocks[size_of_chunk];
    int changes[100];

    chunk(){}

    chunk(int chunk_x, int chunk_z, chunk* chunk_z_minus, chunk* chunk_x_plus, chunk* chunk_z_plus, chunk* chunk_x_minus) :
        chunk_x(chunk_x),
        chunk_z(chunk_z),
        chunk_z_minus(chunk_z_minus),
        chunk_x_plus(chunk_x_plus),
        chunk_z_plus(chunk_z_plus),
        chunk_x_minus(chunk_x_minus),
        up_ptr(0),
        right_ptr(0),
        down_ptr(0),
        left_ptr(0)
    {
        if(chunk_z_minus != nullptr){
            chunk_z_minus->chunk_z_plus = this;
        }
        if(chunk_x_plus != nullptr){
            chunk_x_plus->chunk_x_minus = this;
        }
        if(chunk_z_plus != nullptr){
            chunk_z_plus->chunk_z_minus = this;
        }
        if(chunk_x_minus != nullptr){
            chunk_x_minus->chunk_x_plus = this;
        }
        std::cout << "creation of chunk: " << chunk_x << " " << chunk_z << std::endl;
    }
    /*~chunk(){
        if (chunk_z_minus != nullptr){
            chunk_x_minus->chunk_z_plus = nullptr;
        }
        if(chunk_x_plus != nullptr){
            chunk_x_plus->chunk_x_minus = nullptr;
        }
        if(chunk_z_plus != nullptr){
            chunk_z_plus->chunk_z_minus = nullptr;
        }
        if(chunk_x_minus != nullptr){
            chunk_x_minus->chunk_x_plus = nullptr;
        }

    }*/
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

void print_chunk_blocks(chunk *chunk){
    uint8_t *chachar = reinterpret_cast<uint8_t*>(chunk->chunk_blocks);
    for (int i = 0; i < size_of_chunk; i++){
        if(i != 0 && i % width_of_chunk*width_of_chunk == 0){
            std::cout << std::endl;
        }
        std::cout << static_cast<unsigned int>(chachar[i]) << " ";
    }
    std::cout << std::endl;
}


bool file_exists(const std::string& filename){
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

chunk* create_chunk(int x, int z, Chunk_Manager &hashed_chunks){
    chunk *new_chunk = new chunk(x, z, hashed_chunks.find_chunk(x,   z-1), \
                                       hashed_chunks.find_chunk(x+1, z),   \
                                       hashed_chunks.find_chunk(x,   z+1), \
                                       hashed_chunks.find_chunk(x-1, z));
    hashed_chunks.add_chunk(x, z, new_chunk);
    return(new_chunk);
}

chunk* load_chunk(std::string &base, int x, int z, Chunk_Manager &hashed_chunks, bool end){
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
        std::cout << "size of file is: " << file_size << std::endl;
        if (ec) {
            std::cerr << "ERROR WHEN GETING LENGHT OF FILE: " << filename << " in file: " << __FILE__ << ":" << __LINE__ << " " << ec.message() << std::endl;
        }
        file.read(loaded_file, file_size);
        std::cout << "size of file: " << file_size << std::endl;

        //std::cout << "file content: " << loaded_file << "<-END" << std::endl;

        chunk *new_chunk = create_chunk(x, z, hashed_chunks);
        std::memcpy(new_chunk->chunk_blocks, reinterpret_cast<chunk*>(loaded_file), file_size);
        std::cout << "content: ";
        for(u_int64_t i = 0; i < file_size; i++){
            break;
            std::cout << (&new_chunk->chunk_blocks[i]) << std::endl;
        }
        file.close();
        return new_chunk;

    }
    else {
        std::cout << "file not exists: " << filename << std::endl;
        chunk *new_chunk = create_chunk(x, z, hashed_chunks);
        return new_chunk;
    }
}

void unload_chunk(std::string &base, chunk* chunk, Chunk_Manager &hashed_chunks, std::atomic<uint16_t> *active_saving_chunk_threads){
    std::cout << "start saving chunk: " << chunk->chunk_x << " " << chunk->chunk_z << std::endl;
    std::string filename = base + std::to_string(chunk->chunk_x) + "." + std::to_string(chunk->chunk_z) + ".chunk";
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()){
        std::cout << "ERROR WHEN OPENING FILE" << std::endl;
        exit(-1);
    }
    char *chachar = reinterpret_cast<char*>(&chunk->chunk_blocks);
    file.write(chachar, size_of_chunk);
    for(int i = 0; i < size_of_chunk; i++){
        break;
        file << chachar[i];
    }
    hashed_chunks.delete_chunk(chunk->chunk_x, chunk->chunk_z);
    std::cout << "deleting chunk: " << chunk->chunk_x << " " << chunk->chunk_z << std::endl;
    delete chunk;
    file.close();
    active_saving_chunk_threads->fetch_sub(1);
}

int main(){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, UINT8_MAX);
    std::cout << "first random number: " << dis(gen) << std::endl;

    Chunk_Manager hashed_chunks;

    std::string base = "./data/";
    chunk *test;
    std::atomic<uint16_t> active_saving_chunk_threads = 0;

    int test_size = 100;
    for(int i = 0; i < test_size; i++){
        for(int y = 0; y < test_size; y++){
            load_chunk(base, i, y, hashed_chunks, 0);
        }
    }

    test = hashed_chunks.find_chunk(0, 0);;

    char *chachar = reinterpret_cast<char*>(hashed_chunks.find_chunk(0, 0)->chunk_blocks);
    for(int i = 0; i < size_of_chunk; i++){
        chachar[i] = dis(gen);
    }
    //print_chunk_blocks(hashed_chunks.find_chunk(0, 0));

    //testing
    /*for(int i = 0; i < test_size; i++){
        test = hashed_chunks.find_chunk(0, i);
        for (int y = 0; y < test_size-1; y++){
            std::cout << test->chunk_x << test->chunk_z << " ";
            test = test->chunk_x_plus;
        }
        for (int y = 0; y < test_size-1; y++){
            std::cout << test->chunk_x << test->chunk_z << " ";
            test = test->chunk_x_minus;
        }
        std::cout << test->chunk_x << test->chunk_z << std::endl;
    }
    std::cout << std::endl;
    for(int i = 0; i < test_size; i++){
        test = hashed_chunks.find_chunk(i, 0);
        for (int y = 0; y < test_size-1; y++){
            std::cout << test->chunk_x << test->chunk_z << " ";
            test = test->chunk_z_plus;
        }
        for (int y = 0; y < test_size-1; y++){
            std::cout << test->chunk_x << test->chunk_z << " ";
            test = test->chunk_z_minus;
        }
        std::cout << test->chunk_x << test->chunk_z << std::endl;
    }*/

    for(int i = 0; i < test_size; i++){
        for(int y = 0; y < test_size; y++){
            active_saving_chunk_threads.fetch_add(1);
            std::thread saving_thread(unload_chunk, std::ref(base), hashed_chunks.find_chunk(i, y), std::ref(hashed_chunks), &active_saving_chunk_threads);
            saving_thread.detach();

            continue;
            delete (hashed_chunks.find_chunk(i, y));
            hashed_chunks.delete_chunk(i, y);
        }
    }
    while(active_saving_chunk_threads){}
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
