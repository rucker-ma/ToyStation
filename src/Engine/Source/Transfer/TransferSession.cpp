#include "TransferSession.h"
namespace toystation {

class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver* Create() {
        return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
    }
    virtual void OnSuccess() {
        // LOG(INFO) << __FUNCTION__;
    }
    virtual void OnFailure(webrtc::RTCError error) {
        // LOG(INFO) << __FUNCTION__ << " " << ToString(error.type()) << ": "
        //	<< error.message();
    }
};

void TransferSession::CreateOffer() {
    peer_connection_->CreateOffer(
        this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}
void TransferSession::SetRemoteAnswer(
    webrtc::SessionDescriptionInterface* sdp) {
    peer_connection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create(), sdp);
}
void TransferSession::SetRemoteIceCandidate(
    webrtc::IceCandidateInterface* candidate) {
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
    const webrtc::IceCandidateInterface* candidate) {}
void TransferSession::OnIceConnectionReceivingChange(bool receiving) {}
void TransferSession::OnIceCandidateError(const std::string& host_candidate,
                                          const std::string& url,
                                          int error_code,
                                          const std::string& error_text) {}
void TransferSession::OnNegotiationNeededEvent(uint32_t event_id) {}
void TransferSession::OnSuccess(webrtc::SessionDescriptionInterface* desc) {}
void TransferSession::OnFailure(webrtc::RTCError error) {}
}  // namespace toystation