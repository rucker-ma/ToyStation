#pragma once

#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>

namespace toystation {
class SessionCreator {
public:
    void Initialize();

private:
    void SetupFactory();
    void SetupTracks();
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
        peer_connection_factory_;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;

    std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> config_;
};
}  // namespace toystation