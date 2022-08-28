#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

namespace MoThreadUtils
{
    struct ThreadData 
    {
        std::mutex mtx_;
        std::condition_variable cond_;
        bool is_shutdown_ = false;
        std::queue<std::function<void()>> tasks_;
    };

    class Thread
    {
        public:
            Thread(uint32_t id, std::shared_ptr<ThreadData> thread_data)
            : thread_id_(id), data_(thread_data)
            {
                worker_ = std::thread(&Thread::ThreadLoop, this);
            }

            ~Thread()
            {
                if (worker_.joinable())
                {
                    {
                        std::unique_lock<std::mutex> lk(data_->mtx_);
                        data_->cond_.wait(lk, [this]() { return data_->tasks_.empty(); });
                    }
                    worker_.join();
                }
            }

            inline uint32_t GetThreadId() { return thread_id_; };

        private:
            void ThreadLoop()
            {
                for (;;) 
                {
                    std::function<void()> current;
                    {
                        std::unique_lock<std::mutex> lk(data_->mtx_);
                        data_->cond_.wait(lk, [this]()
                        {
                            return !data_->tasks_.empty() || data_->is_shutdown_; 
                        });
                        if (data_->is_shutdown_)
                        {
                            break;
                        }
                        current = data_->tasks_.front();
                        data_->tasks_.pop();
                    }
                    
                    current();
                }
            }

        private:
            uint32_t thread_id_;
            std::shared_ptr<ThreadData> data_;
            std::thread worker_;
    };

    class ThreadPool 
    {
    public:
        explicit ThreadPool(size_t count)
            : data_(std::make_shared<ThreadData>())
        {
            for (size_t i = 0; i < count; ++i)
            {
                workers.emplace_back(std::make_unique<Thread>(i, data_));
            }
        }

        ThreadPool() = default;
        ThreadPool(ThreadPool&&) = default;

        ~ThreadPool()
        {
            if ((bool)data_)
            {
                {
                    std::lock_guard<std::mutex> lk(data_->mtx_);
                    data_->is_shutdown_ = true;
                }
                data_->cond_.notify_all();
            }
        }

        template <class F>
        void AddTask(F && task) 
        {
            {
                std::lock_guard<std::mutex> lk(data_->mtx_);
                data_->tasks_.emplace(std::forward<F>(task));
            }
            data_->cond_.notify_one();
        }

    private:
        std::shared_ptr<ThreadData> data_;
        std::vector<std::unique_ptr<Thread>> workers;
    };
}


