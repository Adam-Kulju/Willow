#include <string>
#include <thread>
#include <vector>

void run(int thread_id, int threads) {
#ifdef _WIN32
  std::string command =
      std::string(".\\datagen.exe data") + std::to_string(thread_id) + ".txt " + std::to_string(threads);
#else
  std::string command =
      std::string("./datagen data") + std::to_string(thread_id) + ".txt " + std::to_string(threads);
#endif
  std::system(command.c_str()); // runs an instance of Willow for each thread
                                // devoted to datagen.
}

int main(int argc, char *argv[]) {
  std::vector<std::thread> threads;
  for (int i = 0; i < atoi(argv[1]); ++i)
    threads.push_back(std::thread(run, i, atoi(argv[1])));
  for (auto &t : threads)
    t.join();
  return 0;
}
