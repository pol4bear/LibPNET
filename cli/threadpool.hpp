#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>

namespace pol4b {

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    template<class F>
    auto enqueue(F&& f) -> std::future<decltype(f())>;

private:
    void worker();

    std::vector<std::thread> workers;
    std::queue<std::packaged_task<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

ThreadPool::ThreadPool() : stop(false) {
    size_t thread_count = std::thread::hardware_concurrency();
    for (size_t i = 0; i < thread_count; ++i) {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

template<class F>
auto ThreadPool::enqueue(F&& f) -> std::future<decltype(f())> {
    using return_type = decltype(f());

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

void ThreadPool::worker() {
    while (true) {
        std::packaged_task<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

}
