//
// Created by ma on 2023/4/25.
//
#include "Time.h"

#ifdef _WIN64
#include <Windows.h>
#endif
#include "Base/Logger.h"
namespace toystation {
namespace time {
std::string Now() {
    SYSTEMTIME st = {0};
    GetLocalTime(&st);
    return std::to_string(st.wHour) + ":" + std::to_string(st.wMinute) + ":" +
           std::to_string(st.wSecond) + "." + std::to_string(st.wMilliseconds);
}
}  // namespace time
namespace debug {
TimePiling& TimePiling::Instance() {
    static TimePiling piling;
    return piling;
}
void TimePiling::Mark(std::string label,int sequence_number) {
    counts_[label]++;
    if (counts_[label] % duration_ == 0) {
        if(!points_.empty()){
            if(points_.back().sequence>=sequence_number){
                result_.clear();
                for (auto& mk : points_) {
                    result_ = result_ + mk.label + " : " + mk.time + "\n";
                }
                points_.clear();
            }
        }
        points_.push_back({label, time::Now(),sequence_number});
    }
}
std::string TimePiling::GetMarksInfo() {
    auto res = result_;
    result_.clear();
    return res;
}
void TimePiling::ClearCount(){
    for(auto& key_count:counts_){
        key_count.second = 0;
    }
}
TimePiling::TimePiling() : duration_(300) {}
}  // namespace debug
}  // namespace toystation