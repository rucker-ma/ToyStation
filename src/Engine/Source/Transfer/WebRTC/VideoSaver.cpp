//
// Created by ma on 2023/5/2.
//
#include "VideoSaver.h"
#include "Base/Logger.h"
#include "ToyEngineSetting.h"

namespace toystation{

void VideoSaver::Init(const webrtc::VideoCodec* codec_settings){
    const char* filename = "testvideo.flv";
    avformat_alloc_output_context2(&oc_, nullptr,NULL,filename);
    if(!oc_){
        LogError("Cann't alloc avformatcontext");
        return;
    }
    out_stream_ = avformat_new_stream(oc_,NULL);
    out_stream_->time_base = { 1, kFrameRate };

    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    ctx_ = avcodec_alloc_context3(codec);
    ctx_->width = codec_settings->width;
    ctx_->height = codec_settings->height;
    ctx_->pix_fmt = AV_PIX_FMT_NV12;
    ctx_->gop_size = codec_settings->H264().keyFrameInterval;
    ctx_->max_b_frames = 0;
    ctx_->has_b_frames =0;
    ctx_->time_base = {1,kFrameRate};
    ctx_->framerate = {kFrameRate,1};
    int ret = avcodec_open2(ctx_,codec, nullptr);
    if(ret<0){
        LogError("Open Codec Error");
        avformat_free_context(oc_);
        return;
    }
    ret = avcodec_parameters_from_context(out_stream_->codecpar,ctx_);
    if(ret<0){
        LogError("Set codecpar error");
    }
    av_dump_format(oc_,0,filename,1);
    if(!(oc_->oformat->flags& AVFMT_NOFILE)){
        ret = avio_open(&oc_->pb,filename,AVIO_FLAG_WRITE);
        if(ret<0){
            LogError("Could not open ", filename);
        }
    }
    AVDictionary *opt = NULL;
    ret = avformat_write_header(oc_,&opt);
    assert(ret >=0);
    index_ = 0;
}
void VideoSaver::WritePacket(std::vector<uint8_t>& data,std::vector<uint8_t> extra_data){
    pkt = av_packet_alloc();
    void* malloc_data = av_malloc(data.size());
    memcpy(malloc_data,data.data(),data.size());
    int ret = av_packet_from_data(pkt,(uint8_t*)malloc_data,data.size());
    if(ret!=0){
        LogError("initialize avpacket error");
    }
    if(!extra_data.empty()){
         uint8_t* extra_ptr = av_packet_new_side_data(pkt,AV_PKT_DATA_NEW_EXTRADATA,extra_data.size());
         memcpy(extra_ptr,extra_data.data(),extra_data.size());
         assert(ret>=0);
    }
    pkt->stream_index =  out_stream_->index;
    pkt->dts = index_;
    pkt->pts = pkt->dts;
    av_packet_rescale_ts(pkt,ctx_->time_base,out_stream_->time_base);
    ret = av_write_frame(oc_,pkt);
    if(ret<0){
        LogError("avpacket write frame error");
    }
    if(!extra_data.empty()){
        av_packet_free_side_data(pkt);
    }
    av_packet_free(&pkt);
    index_++;
}
void VideoSaver::End(){
    int ret = av_write_trailer(oc_);
    if(ret<0){
        LogError("write trailer error");
    }
    if(!(oc_->oformat->flags& AVFMT_NOFILE)) {
        ret = avio_closep(&oc_->pb);
        assert(ret == 0);
    }
    avformat_free_context(oc_);
    avcodec_free_context(&ctx_);
}
}