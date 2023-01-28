#pragma once

#include <api/create_peerconnection_factory.h>
#include <api/task_queue/default_task_queue_factory.h>
#include <rtc_base/logging.h>

#include "SessionCreator.h"
#include "TransferSession.h"
#include "WebSocket/WebSocketServer.h"

namespace toystation {

class WebRtcLogSink : public rtc::LogSink {
public:
    virtual ~WebRtcLogSink() {}
    void OnLogMessage(const std::string& message,
                      rtc::LoggingSeverity severity) override;
    void OnLogMessage(const std::string& message) override;
};

class TransferSystem {
public:
    void Initialize();

    void OnUserConnect(websocketpp::connection_hdl hdl);
    void OnUserMessage(
        websocketpp::connection_hdl hdl,
        websocketpp::server<websocketpp::config::asio>::message_ptr msg);

private:
    WebRtcLogSink rtc_log_;
    std::shared_ptr<SessionCreator> session_creator_;
    std::shared_ptr<TransferSession> session_;

    std::shared_ptr<SocketServer> session_server_;
};
}  // namespace toystation