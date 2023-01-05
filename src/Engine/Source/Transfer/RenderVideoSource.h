#pragma once
#include <media/base/adapted_video_track_source.h>

namespace toystation
{
    class RenderVideoSource:public rtc::AdaptedVideoTrackSource
    {
        public:
        void Initialize();
    };
}