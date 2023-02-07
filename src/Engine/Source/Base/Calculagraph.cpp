#include "Calculagraph.h"
namespace toystation {
Calculagraph::Calculagraph(std::string name, int interval, double multiply)
    : name_(std::move(name)),
      count_(0),
      interval_(interval),
      multiply_(multiply) {}
void Calculagraph::Start() {
    previous_time_ = std::chrono::steady_clock::now();
}
void Calculagraph::Reset() {
    count_ = 0;
    previous_time_ = std::chrono::steady_clock::now();
}
void Calculagraph::Step() {
    ++count_;

    if (count_ % interval_ == 0) {
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::milliseconds duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time - previous_time_);

        result_ = count_ * multiply_ / static_cast<double>(duration.count());
        OnEnd(result_, duration.count());
        Reset();
    }
}

}  // namespace toystation