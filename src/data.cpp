using namespace std;
#include <vector>
#include <thread>

void run(){
    std::system("./datagen");
}

int main(int argc, char *argv[]){
    std::vector<std::thread> threads;
    for (int i = 0; i < atoi(argv[1]); ++i)
        threads.push_back(std::thread(run));

    // do some other stuff

    // loop again to join the threads
    for (auto &t : threads)
        t.join();
    return 0;
}