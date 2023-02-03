#pragma once

#include <string>

namespace toystation{

    class SignalingMessage{
        public:
        static std::string Welcome();
        static std::string GetType(const std::string& message);
        static std::string GetPayload(const std::string& message);
        static std::string GenCandidate(std::string mid,int index,std::string content,std::string username);
    };
}