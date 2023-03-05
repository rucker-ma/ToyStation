#pragma once
#include <chrono>
#include <functional>
#include <string>

namespace toystation {

class Calculagraph {
public:
    /// @brief Constructor, 计算方式 interval*multiply/duration(ms)
    /// @param name 设置对象名称，方便区分和debug
    /// @param interval 每隔多少个step计算一次
    /// @param multiply 乘数系数，用于调整时间系数(ms,s..)
    Calculagraph(std::string name, int interval = 1, double multiply = 1);

    void Start();
    void Reset();
    void Step();

    std::function<void(double value, long long duration)> OnEnd;

private:
    std::string name_;
    std::chrono::time_point<std::chrono::steady_clock> previous_time_;
    unsigned long count_;
    int interval_;
    double multiply_;
    double result_;
};

}  // namespace toystation