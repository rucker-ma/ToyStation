#include "WebSocketServer.h"

#include <Base/Macro.h>

namespace toystation {

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

constexpr uint16_t LISTEN_PORT = 9002;

void SocketServer::Init() {
    server_.set_access_channels(websocketpp::log::alevel::all);
    server_.clear_access_channels(websocketpp::log::alevel::frame_payload);
    server_.init_asio();
    server_.set_reuse_addr(true);
    server_.set_message_handler(
        websocketpp::lib::bind(&SocketServer::OnMessage, this, _1, _2));
    server_.set_open_handler(
        websocketpp::lib::bind(&SocketServer::OnOpen, this, _1));
    server_.set_close_handler(
        websocketpp::lib::bind(&SocketServer::OnClose, this, _1));

    server_.listen(LISTEN_PORT);
    server_.start_accept();
    server_.run();
}
void SocketServer::OnOpen(websocketpp::connection_hdl hdl) {
    connections_.insert(hdl);
    OnOpenEvent(hdl);
}
void SocketServer::OnClose(websocketpp::connection_hdl hdl) {
    connections_.erase(hdl);
}
void SocketServer::OnMessage(websocketpp::connection_hdl hdl, message_ptr msg) {
    LogInfo("get socket server msg: ", msg->get_payload());
    OnMessageEvent(hdl,msg);
}
void SocketServer::Send(websocketpp::connection_hdl hdl, std::string msg) {
    server_.send(hdl,msg,websocketpp::frame::opcode::TEXT);
}
}  // namespace toystation