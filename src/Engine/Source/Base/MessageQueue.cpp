#include "MessageQueue.h"

namespace toystation {
ThreadSafeQueue::ThreadSafeQueue(ThreadSafeQueue&& queue) {
    msg_queue_ = std::move(queue.msg_queue_);
}

ThreadSafeQueue& ThreadSafeQueue::operator=(ThreadSafeQueue&& queue) {
    if (&queue == this){ return *this;}

    msg_queue_ = std::move(queue.msg_queue_);
    return *this;
}
void ThreadSafeQueue::Push(std::shared_ptr<Msg>&& msg) {
    {
        std::unique_lock<std::mutex> lck(mtx_);
        if(msg_queue_.size()>=max_size_)
        {
            msg_queue_.pop_front();
        }
        msg_queue_.push_back(std::forward<std::shared_ptr<Msg>>(msg));
    }
    cv_.notify_one();
}

bool ThreadSafeQueue::Pop(std::shared_ptr<Msg>& msg) {
    std::unique_lock<std::mutex> lck(mtx_);
    if (msg_queue_.empty()) {
        cv_.wait(lck);
    }
    msg = msg_queue_.front();
    msg_queue_.pop_front();
    return true;
}
void MessageQueue::Post(std::thread::id dst, std::shared_ptr<Msg>&& msg) {
    if (queues_.find(dst) == queues_.end()) {
        std::unique_lock<std::mutex> lck(create_mtx_);
        queues_.insert(std::make_pair(dst, ThreadSafeQueue()));
    }
    queues_.at(dst).Push(std::forward<std::shared_ptr<Msg>>(msg));
}
bool MessageQueue::Get(std::shared_ptr<Msg>& msg) {
    auto thread_id = std::this_thread::get_id();
    if (queues_.find(thread_id) == queues_.end()) {
        std::unique_lock<std::mutex> lck(create_mtx_);
        queues_.insert(std::make_pair(thread_id, ThreadSafeQueue()));
        return false;
    }
    if (queues_.at(thread_id).Pop(msg)) {
        return true;
    }

    return false;
}

}  // namespace toystation