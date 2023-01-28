#include "TransferSystem.h"

#include <rtc_base/ssl_adapter.h>

#include "Base/Macro.h"

#ifdef _WIN32
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dmoguids.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "msdmo.lib")
#pragma comment(lib, "Strmiids.lib")
#endif
namespace toystation {
void WebRtcLogSink::OnLogMessage(const std::string& message,
                                 rtc::LoggingSeverity severity) {
    switch (severity) {
        case rtc::LS_VERBOSE:
            LogDebug(message);
            break;
        case rtc::LS_INFO:
            LogInfo(message);
            /* code */
            break;
        case rtc::LS_WARNING:
            LogWarn(message);
            break;
        case rtc::LS_ERROR:
            LogError(message);
            break;
        default:
            break;
    }
}
void WebRtcLogSink::OnLogMessage(const std::string& message) {
    LogInfo(message);
}

void TransferSystem::Initialize() {
    rtc::LogMessage::AddLogToStream(&rtc_log_, rtc::LS_WARNING);
    rtc::InitializeSSL();

    session_creator_ = std::make_shared<SessionCreator>();
    session_creator_->Initialize();

    session_server_ = std::make_shared<SocketServer>();
    session_server_->OnOpenEvent =
        std::bind(&TransferSystem::OnUserConnect, this, std::placeholders::_1);
    session_server_->OnMessageEvent =
        std::bind(&TransferSystem::OnUserMessage, this, std::placeholders::_1,
                  std::placeholders::_2);
    session_server_->Init();

    // session_ = session_creator_->CreateSession();

    // create peerconnection
}

void TransferSystem::OnUserConnect(websocketpp::connection_hdl hdl) {

    session_server_->Send(hdl,"Welcome");

}

void TransferSystem::OnUserMessage(
    websocketpp::connection_hdl hdl,
    websocketpp::server<websocketpp::config::asio>::message_ptr msg) {

   std::string payload =  msg->get_payload();

    if(payload == "JOIN"){
        session_ = session_creator_->CreateSession(std::make_unique<SessionClient>(hdl,session_server_));
        session_->CreateOffer();
    }
}

}  // namespace toystation
