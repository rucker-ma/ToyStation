#pragma once

#include "SessionCreator.h"
#include "TransferSession.h"
#include "WebSocket/WebSocketServer.h"

namespace toystation {

class TransferSystem {
public:
    void Initialize();

    void OnUserConnect(websocketpp::connection_hdl hdl);
    void OnUserMessage(
        websocketpp::connection_hdl hdl,
        websocketpp::server<websocketpp::config::asio>::message_ptr msg);
    void OnUserClose(websocketpp::connection_hdl hdl);
    bool AnyConnected();
private:
    void Run();

private:
    std::shared_ptr<SessionCreator> session_creator_;
    std::shared_ptr<SocketServer> session_server_;
    std::shared_ptr<std::thread> sserver_thread_;
    std::shared_ptr<std::thread> msg_thread_;

    std::map<websocketpp::connection_hdl, std::shared_ptr<TransferSession>,
             std::owner_less<websocketpp::connection_hdl>>
        session_container_;
};
}  // namespace toystation