#pragma once
#include <chrono>
#include <functional>
#include <string>

namespace toystation {

class Calculagraph {
public:
    Calculagraph(std::string name,int interval = 1, double multiply = 1);

    void Start();
    void Reset();
    void Step();
    //void SetResetAfterEnd(bool enable)
    std::function<void(double value,long long duration)> OnEnd;

private:
    std::string name_;
    std::chrono::time_point<std::chrono::steady_clock> previous_time_;
    unsigned long count_;
    int interval_;
    double multiply_;
    double result_;
};

}  // namespace toystation