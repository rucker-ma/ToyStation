#pragma once
#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>
namespace toystation {
class TransferSession : public webrtc::PeerConnectionObserver,
                        webrtc::CreateSessionDescriptionObserver {
public:
    void CreateOffer();
    void SetRemoteAnswer(webrtc::SessionDescriptionInterface* sdp);
    void SetRemoteIceCandidate(webrtc::IceCandidateInterface* candidate);


    void AddSinkToTack();
    //
    // PeerConnectionObserver implementation.
    //
    void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state) override;
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override;
    void OnRemoveTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
    void OnDataChannel(
        rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnRenegotiationNeeded() override;
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    void OnIceCandidate(
        const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override;
    void OnIceCandidateError(const std::string& host_candidate,
                             const std::string& url, int error_code,
                             const std::string& error_text) override;
    void OnNegotiationNeededEvent(uint32_t event_id) override;
    // CreateSessionDescriptionObserver implementation.
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(webrtc::RTCError error) override;

    void AddRef() const override{};
    rtc::RefCountReleaseStatus Release() const override {
        return rtc::RefCountReleaseStatus::kDroppedLastRef;
    };

protected:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    rtc::scoped_refptr<webrtc::DataChannelInterface> datachl_interface_;
    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> tracks_;
};

}  // namespace toystation