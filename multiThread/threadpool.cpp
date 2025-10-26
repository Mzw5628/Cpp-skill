#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

class MyThreadPool {
 public:
  MyThreadPool(int n = std::thread::hardware_concurrency()) : stop(false) {
    for (int i = 0; i < n; i++) {
      workers.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
          }
          task();
        }
      });
    }
  }
  void enqueue(std::function<void()> f) {
    {
      std::lock_guard<std::mutex> lock(mtx);
      tasks.emplace(std::move(f));
    }
    cv.notify_one();
  }

  template <typename F, typename... Args>
  auto submit(F&& f, Args&&... args) {
    using R = std::invoke_result_t<F, Args...>;
    auto bound = [f = std::forward<F>(f), ... args = std::forward<Args>(args)] {
      return std::invoke(std::move(f), std::move(args)...);
    };
    auto task = std::make_shared<std::packaged_task<R()>>(std::move(bound));
    std::future<R> fut = task->get_future();
    {
      std::lock_guard<std::mutex> lock(mtx);
      tasks.emplace([task = std::move(task)] { (*task)(); });
    }
    cv.notify_one();
    return fut;
  }
  
  ~MyThreadPool() {
    {
      std::lock_guard<std::mutex> lock(mtx);
      stop = true;
    }
    cv.notify_all();
    for (auto& t : workers) t.join();
  }

 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::condition_variable cv;
  std::mutex mtx;
  bool stop;
};


int add(int a, int b) {
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  return (a + b);
}

int main() {
  MyThreadPool pool(3);
  std::vector<std::future<int>> futs;
  for (int i = 0; i < 6; i++) {
    pool.enqueue([i] {
      printf("任务%d正在被线程%d执行\n", i + 1, std::this_thread::get_id());
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      printf("任务%d被线程%d执行完毕\n", i + 1, std::this_thread::get_id());
    });
    futs.emplace_back(pool.submit(add, i, i));
  }
  for (auto& f : futs) {
    int v = f.get();  // 只能 get 一次
    printf("submit返回: %d\n", v);
  }
  printf("program finished\n");
}