#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class MyThreadPool {
 private:
  std::mutex tex;
};

int main() {
  MyThreadPool pool(3);
  for (int i = 0; i < 6; i++) {
    pool.enqueue([i] {
      printf("任务%d正在被线程%d执行\n", i + 1, std::this_thread::get_id());
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      printf("任务%d被线程%d执行完毕\n", i + 1, std::this_thread::get_id());
    });
  }
  printf("program finished\n");
}