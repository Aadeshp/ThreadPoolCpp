#include "thread_pool.hpp"

#include <iostream>

using namespace ap;

thread_pool::thread_pool(const size_t size) :
    num_free_threads_(0),
    is_done_(false)
{
    this->set_pool_size(size);
}

thread_pool::~thread_pool() {
    this->drain();
}

void thread_pool::drain() {
    if (this->is_done_) {
        return;
    }

    this->is_done_ = true;

    {
        std::unique_lock<std::mutex> lock(this->mutex_);
        this->cv_.notify_all();
    }

    for (auto& thread : this->threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    this->threads_.clear();
}

const size_t thread_pool::get_num_threads() const {
    return this->threads_.size();
}

void thread_pool::set_pool_size(const size_t size) {
    if (this->is_done_) {
        return;
    }

    for (size_t i = 0; i < size; ++i) {
        this->run_thread(i);
    }
}

void thread_pool::run_thread(const int i) {
    auto f = [this] {
        std::function<void()> job;

        while (true) {
            while (!this->q_.empty()) {
                job = std::move(this->q_.pop());
                job();
            }

            std::unique_lock<std::mutex> lock(this->mutex_);
            ++this->num_free_threads_;
            this->cv_.wait(lock, [this]() { return this->is_done_ || !this->q_.empty(); });
            --this->num_free_threads_;

            if (is_done_ && this->q_.empty()) {
                return;
            }
        }
    };

    this->threads_.emplace_back(f);
}
