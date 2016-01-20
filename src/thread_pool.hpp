#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <memory>
#include <atomic>
#include <functional>
#include <exception>
#include <cstdlib>

namespace ap {
namespace internal {
    template <typename T>
    class safe_queue final {
        public:
            inline bool push(const T& value) {
                std::unique_lock<std::mutex> lock(this->mutex_);
                this->q_.push(value);

                return true;
            }

            inline T pop() {
                std::unique_lock<std::mutex> lock(this->mutex_);

                T value = this->q_.front();
                this->q_.pop();

                return value;
            }

            inline bool empty() {
                std::unique_lock<std::mutex> lock(this->mutex_);

                return this->q_.empty();
            }

        private:
            std::queue<T> q_;
            std::mutex mutex_;
    };
};

    class thread_pool {
        public:
            thread_pool(const size_t size);
            ~thread_pool();

            const size_t get_num_threads() const;


            template <class F, class... Args>
            inline auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
                auto job = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                );

                this->q_.push([job]() { (*job)(); });

                {
                    std::unique_lock<std::mutex> lock(this->mutex_);
                    this->cv_.notify_one();
                }

                return job->get_future();
            }


            inline auto dequeue() -> std::function<void()> {
                std::function<void()> f;

                if (!this->q_.empty()) {
                    f = this->q_.pop();
                }

                return f;
            }
            
        private:
            void run_thread(const int i);
            void set_pool_size(const size_t size);
            void drain();

            std::vector<std::thread> threads_;
            internal::safe_queue<std::function<void()>> q_;
            
            std::atomic<bool> is_done_;
            std::atomic<size_t> num_free_threads_;

            std::mutex mutex_;
            std::condition_variable cv_;
    };
};

#endif
