#include <vector>
#include <thread>
#include <string>

void run(int thread_id){
#ifdef _WIN32
    std::string command = std::string(".\\datagen.exe data") + std::to_string(thread_id) + ".txt";
#else
    std::string command = std::string("./datagen.exe data") + std::to_string(thread_id) + ".txt";
#endif
    std::system(command.c_str());
}

int main(int argc, char *argv[]){
    std::vector<std::thread> threads;
    for (int i = 0; i < atoi(argv[1]); ++i)
        threads.push_back(std::thread(run, i));

    // do some other stuff

    // loop again to join the threads
    for (auto &t : threads)
        t.join();
    return 0;
}