#pragma once

#include <set>
#include <functional>

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#ifndef _WEBSOCKETPP_CPP11_INTERNAL_
#define _WEBSOCKETPP_CPP11_INTERNAL_
#endif

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace toystation {


class SocketInterface
{
public:

    virtual void Send(websocketpp::connection_hdl hdl,std::string msg){}
};


class SocketServer:public SocketInterface {
    typedef websocketpp::server<websocketpp::config::asio> server;
    typedef server::message_ptr message_ptr;


public:
    void Init();
    void OnOpen(websocketpp::connection_hdl hdl);
    void OnClose(websocketpp::connection_hdl hdl);
    void OnMessage(websocketpp::connection_hdl hdl, message_ptr msg);

    void Send(websocketpp::connection_hdl hdl,std::string msg)override;

    std::function<void(websocketpp::connection_hdl hdl, message_ptr msg)> OnMessageEvent;
    std::function<void(websocketpp::connection_hdl hdl)> OnOpenEvent;
private:
    server server_;
    std::set<websocketpp::connection_hdl,
             std::owner_less<websocketpp::connection_hdl>>
        connections_;
};
}  // namespace toystation