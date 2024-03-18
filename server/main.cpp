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

struct inventory_box{
    block block_type;
    uint8_t block_count;
};

struct player{
    uint32_t location_x;
    uint32_t location_y;
    inventory_box inventory[46];
};

struct chest{
    inventory_box inventory[27];
};

struct chunk{
    int32_t chunk_x;       // 4
    int32_t chunk_z;       // 4
    chunk *chunk_z_minus;  // 8
    chunk *chunk_x_plus;   // 8
    chunk *chunk_z_plus;   // 8
    chunk *chunk_x_minus;  // 8
    uint16_t up_ptr;       // 2
    uint16_t right_ptr;    // 2
    uint16_t down_ptr;     // 2
    uint16_t left_ptr;     // 2
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
        std::cout << "sizeof pointer: " << sizeof(chunk*) << std::endl;
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

void delete_folder(const std::string& folderPath) {
    try {
        // Iterate over all files in the directory
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            // Check if it's a regular file
            if (std::filesystem::is_regular_file(entry.path())) {
                // Delete the file
                std::filesystem::remove(entry.path());
                std::cout << "Deleted file: " << entry.path() << std::endl;
            }
        }
        std::cout << "All files in folder \"" << folderPath << "\" deleted successfully." << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

//slower -> 24s vs 16s
uint32_t get_rand_uint32_t2(){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> distribution(0, UINT32_MAX);
    auto number = distribution(gen);
    return number;
}

//faster -> 16s vs 24s
uint32_t get_rand_uint32_t() {
    // Seed the random number generator with current time
    static uint8_t test = 0;
    if(test == 0){
        std::srand(static_cast<uint32_t>(std::time(nullptr)));
        test++;
    }

    // Generate random number within the range [min, max]
    return (std::rand() % (UINT32_MAX));
}

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
        exit(-1);
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
            exit(-1);
        }
        std::error_code ec{};
        auto file_size = std::filesystem::file_size(filename, ec);
        std::cout << "size of file is: " << file_size << std::endl;
        if (ec) {
            std::cerr << "ERROR WHEN GETING LENGHT OF FILE: " << filename << " in file: " << __FILE__ << ":" << __LINE__ << " " << ec.message() << std::endl;
            exit(-1);
        }
        file.read(loaded_file, file_size);
        std::cout << "size of file: " << file_size << std::endl;

        //std::cout << "file content: " << loaded_file << "<-END" << std::endl;

        chunk *new_chunk = create_chunk(x, z, hashed_chunks);
        std::memcpy(new_chunk->chunk_blocks, reinterpret_cast<chunk*>(loaded_file), file_size);

        file.close();
        return new_chunk;

    }
    else {
        std::cout << "file not exists: " << filename << std::endl;
        chunk *new_chunk = create_chunk(x, z, hashed_chunks);
        char *chachar = reinterpret_cast<char*>(new_chunk->chunk_blocks);
        for(int i = 0; i < size_of_chunk; i++){
            chachar[i] = static_cast <char>(get_rand_uint32_t() & 0xFF);
            //chachar[i] = static_cast <char>(i);
        }
        return new_chunk;
    }
}

void unload_chunk(std::string &base, chunk* chunk, Chunk_Manager &hashed_chunks){
    std::cout << "start saving chunk: " << chunk->chunk_x << " " << chunk->chunk_z << std::endl;
    std::string filename = base + std::to_string(chunk->chunk_x) + "." + std::to_string(chunk->chunk_z) + ".chunk";
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()){
        std::cout << "ERROR WHEN OPENING FILE" << std::endl;
        exit(-1);
    }
    char *chachar = reinterpret_cast<char*>(&chunk->chunk_blocks);
    file.write(chachar, size_of_chunk);
    hashed_chunks.delete_chunk(chunk->chunk_x, chunk->chunk_z);
    std::cout << "deleting chunk: " << chunk->chunk_x << " " << chunk->chunk_z << std::endl;
    delete chunk;
    file.close();
}

int main(){

    static std::stringstream stdout_buffer;;
    //std::setbuf(stdout, stdout_buffer);
    //std::setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));
    //std::cout << std::nounitbuf;
    std::streambuf *old = std::cout.rdbuf(stdout_buffer.rdbuf());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, UINT32_MAX);
    std::cout << "first random number: " << dis(gen) << std::endl;

    delete_folder("./data");

    Chunk_Manager hashed_chunks;

    std::string base = "./data/";
    chunk *test;
    std::atomic<uint16_t> active_saving_chunk_threads = 0;

    auto time1 = std::chrono::system_clock::now();
    int test_size = 10;
    for(int i = 0; i < test_size; i++){
        for(int y = 0; y < test_size; y++){
            test = load_chunk(base, i, y, hashed_chunks, 0);
            std::cout << "# test: " << test << std::endl;
        }
    }
    auto time2 = std::chrono::system_clock::now();


    for(int i = 0; i < test_size; i++){
        for(int y = 0; y < test_size; y++){
            //std::thread saving_thread(unload_chunk, std::ref(base), hashed_chunks.find_chunk(i, y), std::ref(hashed_chunks), &active_saving_chunk_threads);
            //saving_thread.detach();
            unload_chunk(base, hashed_chunks.find_chunk(i, y), hashed_chunks);
            //usleep(100);

            continue;
            delete (hashed_chunks.find_chunk(i, y));
            hashed_chunks.delete_chunk(i, y);
        }
    }
    auto time3 = std::chrono::system_clock::now();
    std::cout << "time to create: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time2-time1).count() << " ns" << std::endl;
    std::cout << "time to create: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time2-time1).count()/1000 << "    us" << std::endl;
    std::cout << "time to create: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time2-time1).count()/1000000 << "       ms" << std::endl;
    std::cout << "time to create: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time2-time1).count()/1000000000 << "           s" << std::endl;

    std::cout << "time to save: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time3-time2).count() << " ns" << std::endl;
    std::cout << "time to save: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time3-time2).count()/1000 << "    us" << std::endl;
    std::cout << "time to save: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time3-time2).count()/1000000 << "       ms" << std::endl;
    std::cout << "time to save: " << std::chrono::duration_cast<std::chrono::nanoseconds>(time3-time2).count()/1000000000 << "           s" << std::endl;
    while(active_saving_chunk_threads){}
    if(load_chunk(base, 1, 1, hashed_chunks, 1)){
        std::cout << "ERROR WHEN CLEANING FUNCTION load_chunk " << " in file: " << __FILE__ << ":" << __LINE__ << std::endl;
    }
    std::cout << sizeof(chunk) << " " << sizeof(struct block) << std::endl;
    auto random = get_rand_uint32_t();
    if(random % 2){
        // just to delete data  so I can push it to git
        delete_folder("./data");
        std::cout << "deleting all files in ./data " << random % 2 << " " << random << std::endl;
    }
    std::cout << "END OF PROGRAM #5" << std::endl << std::endl;
    std::cout.rdbuf(old);
    std::cout << stdout_buffer.str();
}

//clear && g++ -fwhole-program -O3 -g -fsanitize=address -fsanitize=leak main.cpp -o main && ./main && g++ ./main.cpp -g -o main && valgrind -s --leak-check=full ./main
//clear && g++ -fwhole-program -O3 -g -fsanitize=address -fsanitize=leak main.cpp -o main && ./main && g++ ./main.cpp -g -o main && valgrind -s --leak-check=full --show-leak-kinds=all ./main

//g++ ./main.cpp -O3 -fwhole-program -mavx2 && time ./main
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


/*Thread 383: status = VgTs_WaitSys syscall 202 (lwpid 34613)
==34134==    at 0x4B4E25B: futex_wait (futex-internal.h:146)
==34134==    by 0x4B4E25B: __lll_lock_wait_private (lowlevellock.c:34)
==34134==    by 0x4B3D094: fwrite (iofwrite.c:37)
==34134==    by 0x49ADB64: std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long) (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.30)
==34134==    by 0x10E09D: unload_chunk(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >&, chunk*, Chunk_Manager&, std::atomic<unsigned short>*) (in /home/nejhate/Desktop/recraft/server/main)
==34134==    by 0x494D252: ??? (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.30)
==34134==    by 0x4B51AC2: start_thread (pthread_create.c:442)
==34134==    by 0x4BE2A03: clone (clone.S:100)
client stack range: [0x22AA05000 0x22B203FFF] client SP: 0x22B203A38
valgrind stack range: [0x109D86F000 0x109D96EFFF] top usage: 3224 of 1048576*/

/*Thread 499: status = VgTs_Init (lwpid 0)
==34788==    at 0x4BE29F6: clone (clone.S:83)
==34788==    by 0x4B517CF: ??? (pthread_create.c:321)
==34788==    by 0x26527863F: ???
client stack range: [0x264A79000 0x265277FFF] client SP: 0x265277EF0
valgrind stack range: [0x10B34B7000 0x10B35B6FFF] top usage: 552 of 1048576*/

/*Thread 1: status = VgTs_Runnable syscall 56 (lwpid 34788)
==34788==    at 0x4BE29F6: clone (clone.S:83)
==34788==    by 0x4BE38EE: __clone_internal (clone-internal.c:83)
==34788==    by 0x4B516D8: create_thread (pthread_create.c:295)
==34788==    by 0x4B521FF: pthread_create@@GLIBC_2.34 (pthread_create.c:828)
==34788==    by 0x494D328: std::thread::_M_start_thread(std::unique_ptr<std::thread::_State, std::default_delete<std::thread::_State> >, void (*)()) (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.30)
==34788==    by 0x10CE1E: main (in /home/nejhate/Desktop/recraft/server/main)
client stack range: [0x1FFEFFB000 0x1FFF000FFF] client SP: 0x1FFEFFC978
valgrind stack range: [0x1002CAE000 0x1002DADFFF] top usage: 18984 of 1048576*/

/*tcache_thread_shutdown(): unaligned tcache chunk detected
Aborted (core dumped)*/

//--max-threads=INT

/*
time to create: 24474016753 ns
time to create: 24474016    us
time to create: 24474       ms
time to create: 24           s
time to save: 2075589333 ns
time to save: 2075589    us
time to save: 2075       ms
time to save: 2           s
*/