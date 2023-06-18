#include "TransferSession.h"

#include <json/json.h>
#include <p2p/base/ice_transport_internal.h>
#include <pc/session_description.h>

#include "Base/Macro.h"
#include "SignalingMessage.h"

namespace toystation {

class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver* Create() {
        return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
    }
    void OnSuccess() override {
        // LOG(INFO) << __FUNCTION__;
    }
    void OnFailure(webrtc::RTCError error) override {
        // LOG(INFO) << __FUNCTION__ << " " << ToString(error.type()) << ": "
        //	<< error.message();
        LogError("Set Sdp error:" + std::string(error.message()));
    }
};

SessionClient::SessionClient(websocketpp::connection_hdl hdl,
                             std::shared_ptr<SocketInterface> server)
    : hdl_(hdl), socket_(server) {}

TransferSession::TransferSession(std::unique_ptr<SessionClient> client)
    : client_(std::move(client)) {}

TransferSession::~TransferSession() {
    if (input_observer_) {
        input_observer_ = nullptr;
    }
    if (peer_connection_) {
        peer_connection_->Close();
        peer_connection_ = nullptr;
    }
}

void TransferSession::SetPeerConnection(
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection) {
    peer_connection_ = peer_connection;
    webrtc::DataChannelInit init;
    auto dc_err = peer_connection_->CreateDataChannelOrError(
        std::string("user_input"), &init);

    if (dc_err.ok()) {
        input_observer_ = std::make_unique<InpuDataChannelObserver>();
        // RegisterObserver at SetDataChannel and UnregisterObserver at
        // destructor
        input_observer_->SetDataChannel(dc_err.value());
    } else {
        LogError("PeerConnect Create Datachannel Error");
    }
}

void TransferSession::CreateOffer() {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    options.offer_to_receive_video = 0;
    options.offer_to_receive_audio = 0;

    peer_connection_->CreateOffer(this, options);
}
void TransferSession::PeerConnectionAddTrack(
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track,
    const std::vector<std::string>& stream_ids) {
    peer_connection_->AddTrack(track, stream_ids);
}
void TransferSession::SetRemoteAnswer(std::string sdp) {
    std::unique_ptr<webrtc::SessionDescriptionInterface> answer =
        webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdp);
    peer_connection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create(), answer.release());
}
void TransferSession::SetRemoteIceCandidate(std::string mid, int index,
                                            std::string sdp) {
    webrtc::SdpParseError error;
    webrtc::IceCandidateInterface* candidate =
        webrtc::CreateIceCandidate(mid, index, sdp, &error);

    peer_connection_->AddIceCandidate(candidate);
}
void TransferSession::AddSinkToTack() {
    std::vector<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
        transceivers = peer_connection_->GetTransceivers();
    for (rtc::scoped_refptr<webrtc::RtpTransceiverInterface>& transceiver :
         transceivers) {
        if (transceiver->media_type() == cricket::MediaType::MEDIA_TYPE_AUDIO) {
            rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver =
                transceiver->receiver();
            rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track =
                receiver->track();
            if (track) {
                webrtc::AudioTrackInterface* audio_track =
                    static_cast<webrtc::AudioTrackInterface*>(track.get());
                // audio_track->AddSink(&AudioSink);
            }
        }
    }
}
void TransferSession::OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState new_state) {}
void TransferSession::OnAddTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
        streams) {}
void TransferSession::OnRemoveTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {}
void TransferSession::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {}
void TransferSession::OnRenegotiationNeeded() {}
void TransferSession::OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState new_state) {}
void TransferSession::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState new_state) {}
void TransferSession::OnIceCandidate(
    const webrtc::IceCandidateInterface* candidate) {
    // TODO:send ice to remote
    LogDebug("send ice to remote");
    std::string candidate_str;
    candidate->ToString(&candidate_str);
    client_->socket_->Send(
        client_->hdl_, SignalingMessage::GenCandidate(
                           candidate->sdp_mid(), candidate->sdp_mline_index(),
                           candidate_str, candidate->candidate().username()));
}
void TransferSession::OnIceConnectionReceivingChange(bool receiving) {}
//void TransferSession::OnIceCandidateError(const std::string& host_candidate,
//                                          const std::string& url,
//                                          int error_code,
//                                          const std::string& error_text) {}
void TransferSession::OnIceCandidateError(const std::string& address,int port,
                         const std::string& url,int error_code,const std::string& error_text){};
void TransferSession::OnNegotiationNeededEvent(uint32_t event_id) {}
void TransferSession::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    std::string sdp;
    desc->ToString(&sdp);
    LogInfo("Local Description: "+ sdp);

    cricket::SessionDescription* desp = desc->description();
    cricket::TransportInfos& infos = desp->transport_infos();
    for (auto& info : infos) {
        info.description.ice_mode = cricket::ICEMODE_LITE;
    }
    desc->ToString(&sdp);

    peer_connection_->SetLocalDescription(
        DummySetSessionDescriptionObserver::Create(), desc);

    auto rtp_senders = peer_connection_->GetSenders();
    for (auto& sender : rtp_senders) {
        webrtc::RtpParameters parameters = sender->GetParameters();
        for (auto& encoding_par : parameters.encodings) {
            // about 10MB for video
            encoding_par.max_bitrate_bps = 80000000;
            encoding_par.min_bitrate_bps = 20000000;
        }
        sender->SetParameters(parameters);
    }

    Json::Value send_remote;
    if (desc->GetType() == webrtc::SdpType::kOffer) {
        send_remote["type"] = "offer";
    }
    if (desc->GetType() == webrtc::SdpType::kAnswer) {
        send_remote["type"] = "answer";
    }
    send_remote["payload"] = sdp;

    client_->socket_->Send(client_->hdl_, send_remote.toStyledString());
}
void TransferSession::OnFailure(webrtc::RTCError error) {}
bool TransferSession::IsConnected() {
    return peer_connection_->peer_connection_state() ==
           webrtc::PeerConnectionInterface::PeerConnectionState::kConnected;
}
}  // namespace toystation