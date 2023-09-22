
#include <vector>
#include <thread>
#include <chrono>
void run(){
    std::system("./datagen");
}

int main(int argc, char *argv[]){
    std::vector<std::thread> threads;
    for (int i = 0; i < atoi(argv[1]); ++i){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        threads.push_back(std::thread(run));
    }

    // do some other stuff

    // loop again to join the threads
    for (auto &t : threads)
        t.join();
    return 0;
}