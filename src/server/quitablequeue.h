#ifndef __batyr_quitablequeue_h_
#define __batyr_quitablequeue_h_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>


namespace Batyr {

    template <typename T>
        class QuitableQueue
        {

            private:
                std::queue<T> queue_;
                std::mutex mutex_;
                std::condition_variable cond_;
                bool continue_;


            public:

                QuitableQueue()
                    : continue_(true)
                {}

                /** disable copying */
                QuitableQueue(const QuitableQueue &) = delete;

                /** disable copying */
                QuitableQueue& operator=(const QuitableQueue &) = delete;

                /**
                 * set the item parameter to the next item in the queue
                 * and return true
                 * or return false if the quit method of the queue has been called
                 *
                 * Will block until any of these event occurs.
                 */
                bool popWait(T& item)
                {
                    if (!continue_) {
                        return false;
                    }

                    std::unique_lock<std::mutex> mlock(mutex_);
                    while (continue_ && queue_.empty())
                    {
                        cond_.wait(mlock);
                    }
                    if (!continue_) {
                        return false;
                    }
                    item = queue_.front();
                    queue_.pop();
                    return true;
                }

                
                /**
                 * pop the nex item from the queue
                 *
                 * will not block and wait for an item to be available
                 * but instead return instantly without modifying the refernce
                 * to the item parameter
                 */
                void popNoWait(T& item)
                {
                    std::unique_lock<std::mutex> mlock(mutex_);
                    if (!queue_.empty()) {
                        item = queue_.front();
                        queue_.pop();
                    }
                }

                /**
                 * quit the queue and signal all consumers waiting
                 * on the pop method
                 */
                void quit()
                {
                    std::unique_lock<std::mutex> mlock(mutex_);
                    continue_ = false;
                    cond_.notify_all();
                }

                /**
                 * add an item to the end of the queue
                 */
                void push(const T& item)
                {
                    std::unique_lock<std::mutex> mlock(mutex_);
                    queue_.push(item);
                    mlock.unlock();
                    cond_.notify_one();
                }

                /**
                 * number of elements waiting in the queue
                 */
                size_t size()
                {
                    return queue_.size();
                }

        };

};

#endif // __batyr_quitablequeue_h_
