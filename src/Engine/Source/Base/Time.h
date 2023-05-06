//
// Created by ma on 2023/4/25.
//

#pragma once
#include <map>
#include <string>
#include <vector>
namespace toystation {

namespace time {

std::string Now();
}
namespace debug {
class TimePiling {
public:
    struct MarkPoint {
        std::string label;
        std::string time;
        int sequence;
    };
    static TimePiling& Instance();
    void Mark(std::string label,int sequence_number);
    std::string GetMarksInfo();
    void ClearCount();
private:
    TimePiling();
    std::vector<MarkPoint> points_;  // 记录数据流通到某一环节的时点
    std::map<std::string, long long> counts_;
    int duration_;
    std::string result_;
};
}  // namespace debug
}  // namespace toystation
