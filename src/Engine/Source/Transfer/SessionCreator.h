#pragma once

#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>

#include "RenderVideoSource.h"
#include "TransferSession.h"

namespace toystation {
class SessionCreator {
public:
    void Initialize();
    std::shared_ptr<TransferSession> CreateSession(std::unique_ptr<SessionClient> client);
private:
    void SetupFactory();
    void SetupTracks();
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
        peer_connection_factory_;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;

     rtc::scoped_refptr<rtc::AdaptedVideoTrackSource> video_source_;

    std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> config_;
};
}  // namespace toystation