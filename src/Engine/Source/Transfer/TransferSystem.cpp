#include "TransferSystem.h"

#include "Base/Global.h"
#include "Base/Macro.h"
#include "SessionCreator.h"
#include "SignalingMessage.h"

#ifdef _WIN32
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dmoguids.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "msdmo.lib")
#pragma comment(lib, "Strmiids.lib")
#endif
namespace toystation {

void TransferSystem::Initialize() {
    session_creator_ = std::make_shared<SessionCreator>();
    session_creator_->Initialize();

    session_server_ = std::make_shared<SocketServer>();
    session_server_->OnOpenEvent =
        std::bind(&TransferSystem::OnUserConnect, this, std::placeholders::_1);
    session_server_->OnMessageEvent =
        std::bind(&TransferSystem::OnUserMessage, this, std::placeholders::_1,
                  std::placeholders::_2);

    session_server_->OnCloseEvent =
        std::bind(&TransferSystem::OnUserClose, this, std::placeholders::_1);

    sserver_thread_ = std::thread([this] { session_server_->Init(); });

    msg_thread_ = std::thread([this] {
        MSG msg;
        BOOL gm;
        while ((gm = GetMessage(&msg, NULL, 0, 0)) != 0 && gm != -1) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    });
    Global::SetTransferThread(std::thread([this] { Run(); }));
}

void TransferSystem::OnUserConnect(websocketpp::connection_hdl hdl) {
    session_server_->Send(hdl, SignalingMessage::Welcome());
}

void TransferSystem::OnUserMessage(
    websocketpp::connection_hdl hdl,
    websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
    std::string message = msg->get_payload();

    if (SignalingMessage::GetType(message) == "join") {
        std::shared_ptr<TransferSession> session =
            session_creator_->CreateSession(
                std::make_unique<SessionClient>(hdl, session_server_));
        session->CreateOffer();
        // when createoffer success, save session to container
        session_container_.insert(std::make_pair(hdl, session));
    }
    if (SignalingMessage::GetType(message) == "answer") {
        if (session_container_.find(hdl) != session_container_.end()) {
            session_container_[hdl]->SetRemoteAnswer(
                SignalingMessage::GetPayload(message));
        }
    }
}
void TransferSystem::OnUserClose(websocketpp::connection_hdl hdl) {
    if (session_container_.find(hdl) != session_container_.end()) {
        session_container_.erase(hdl);
    }
}

void TransferSystem::Run() {
    std::shared_ptr<Msg> msg;
    while (true) {
        if (kMesssageQueue.Get(msg)) {
            if (msg->GetID() == kTransferMessageID) {
                auto* frame_msg =
                    dynamic_cast<DataMsg<RenderFrame>*>(msg.get());
                if (frame_msg) {
                    LogDebug("Receive Render Frame");
                    auto& payload = frame_msg->GetPayload();
                    RenderEvent::OnRenderDone(payload);
                }
            }
        }
    }
}

}  // namespace toystation
