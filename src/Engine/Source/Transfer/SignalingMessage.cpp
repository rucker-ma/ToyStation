#include "SignalingMessage.h"

#include <json/json.h>

namespace toystation {

std::string SignalingMessage::Welcome() {
    Json::Value packet;
    packet["type"] = "welcome";
    return packet.toStyledString();
}
std::string SignalingMessage::GetType(const std::string& message) {
    Json::CharReaderBuilder builder;
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
    std::string error;
    Json::Value result;
    if (!reader->parse(message.c_str(), message.c_str() + message.length(),
                       &result, &error)) {
        return std::string{};
    }
    return result["type"].asString();
}
std::string SignalingMessage::GetPayload(const std::string& message) {
    Json::CharReaderBuilder builder;
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
    std::string error;
    Json::Value result;
    if (!reader->parse(message.c_str(), message.c_str() + message.length(),
                       &result, &error)) {
        return std::string{};
    }
    return result["payload"].asString();
}
std::string SignalingMessage::GenCandidate(std::string mid, int index,
                                           std::string content,
                                           std::string username) {
    Json::Value candidate;
    candidate["sdpMid"] = mid;
    candidate["sdpMLineIndex"] = index;
    candidate["candidate"] = content;
    candidate["usernameFragment"] = username;
    Json::Value packet;
    packet["type"] = "candidate";
    packet["payload"] = candidate.toStyledString();
    return packet.toStyledString();
}
}  // namespace toystation