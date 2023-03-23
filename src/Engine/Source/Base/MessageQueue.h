#pragma once
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <cassert>

namespace toystation {

class Msg {
public:
    virtual ~Msg() = default;
    Msg(int message_id) : msg_id_(message_id){};
    int GetID() const { return msg_id_; }

private:
    int msg_id_;
};

template <typename DataType>
class DataMsg : public Msg {
public:
    template <typename... Args>
    DataMsg(int message_id, Args&&... args)
        : Msg(message_id),
          payload_(new DataType(std::forward<Args>(args)...)) {}


    DataMsg(int message_id, std::unique_ptr<DataType>&& data)
        : Msg(message_id),
          payload_(std::move(data)) {}

    DataMsg(int message_id, DataType* payload)
        : Msg(message_id), payload_(payload) {}
    DataType& GetPayload() const {
        assert(payload_);
        return *payload_;
    }
    std::shared_ptr<DataType> SharedPayload() {
        assert(payload_);
        return std::move(payload_);
    }
    virtual ~DataMsg() {}

private:
    std::unique_ptr<DataType> payload_;
};
class Task{
public:
    virtual ~Task(){};
    virtual void Run()=0;
    virtual void Wait(){}
};

//添加final,避免后面使用时出现菱形继承问题
template<typename T>
class TaskMessage final:public Msg,Task{
public:
    TaskMessage(int message_id, const T& job): Msg(message_id), job_(job){}
    virtual void Run(){
     job_();
     Signal();
    }
    virtual void Wait(){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock,[this]{return signal_;});
        signal_ = false;
    };
    virtual void Reset(){
        signal_ = false;
    }
    virtual void Signal(){
        signal_ = true;
        cv_.notify_all();
    }
private:
    bool signal_;
    std::mutex mtx_;
    std::condition_variable cv_;
    T job_;
};

class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(ThreadSafeQueue&& queue);
    ThreadSafeQueue& operator=(ThreadSafeQueue&& queue);

    void Push(std::shared_ptr<Msg>&& msg);
    bool Pop(std::shared_ptr<Msg>& msg);

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::deque<std::shared_ptr<Msg>> msg_queue_;
    int max_size_ = 5;
};

class MessageQueue {
public:
    void Post(std::thread::id dst, std::shared_ptr<Msg>&& msg);
    bool Get(std::shared_ptr<Msg>& msg);

private:
    std::map<std::thread::id, ThreadSafeQueue> queues_;
    std::mutex create_mtx_;
};

}  // namespace toystation
