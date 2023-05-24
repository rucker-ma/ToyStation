<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from "vue";
import { RtcClient } from "../module/rtc";
import {InputProcess} from "@/module/input_process";

const client = new RtcClient();

const container = ref<HTMLDivElement | null>(null);
const remoteVideo = ref<HTMLVideoElement | null>(null);

const input_process = new InputProcess((msg:JSON)=>{
  client.sendJsonInput(msg);
});


onMounted(() => {
  if (remoteVideo.value) {

    input_process.setupVideoMouseInput(remoteVideo.value);
    input_process.setupKeyboardInput();
  }
});

function connectToServer() {
  client.pc.ontrack = getTrack;
  client.connect();
}
function renderdocCapture() {
  const keyinfo = {
    trigger:"event",
    name:"capture",
    packet:{}
  };
  let content = JSON.stringify(keyinfo);
  client.sendInput(content);
}
function shaderUpdate() {
  const keyinfo = {
    trigger:"event",
    name:"shader",
    packet:{}
  };
  let content = JSON.stringify(keyinfo);
  client.sendInput(content);
}
function getTrack(ev: RTCTrackEvent): void {
  if (ev.track.kind === "video") {
    if (remoteVideo.value) {
      remoteVideo.value.srcObject = ev.streams[0];
      const video_track: MediaStreamTrack = ev.streams[0].getVideoTracks()[0];
      // video_track.onmute = function (ev) {
      //   console.log("mute...");
      // };
      // video_track.onunmute = function (ev) {
      //   console.log("unmute...");
      // };
      remoteVideo.value.addEventListener("error", function (err) {
        console.log("remote video error" + err);
      });
      remoteVideo.value.addEventListener("loadedmetadata", function () {
        remoteVideo.value!.play();
      });
    }
  }
}

const maxWidth = computed(() => {
  if (!remoteVideo.value) return "none";
  const { width, height } = remoteVideo.value;
  const aspectRatio = width / height;
  const containerWidth = container.value!.clientWidth;
  const containerHeight = container.value!.clientHeight;
  const maxHeight = containerHeight;
  const maxWidth = maxHeight * aspectRatio;
  return maxWidth > containerWidth ? containerWidth : "none";
});

const maxHeight = computed(() => {
  if (!remoteVideo.value) return "none";
  const { width, height } = remoteVideo.value;
  const aspectRatio = width / height;
  const containerHeight = container.value!.clientHeight;
  const maxHeight = containerHeight;
  return maxHeight;
});
</script>

<template>
  <div ref="container" class="container">
    <div class="video-container">
      <video
        id="remote-video"
        ref="remoteVideo"
        class="remote-video"
        playsinline
        preload="metadata"
        :style="`max-width: ${maxWidth}px; max-height: ${maxHeight}px; width: 100%; height: 100%;`"
      ></video>
    </div>
    <div class="floating-button-container">
      <el-button @click="connectToServer">Connect</el-button>
    </div>
    <div class="capture-button-container">
      <el-button @click="renderdocCapture">Capture</el-button>
    </div>
    <div class="shader-button-container">
      <el-button @click="shaderUpdate">Shader</el-button>
    </div>
  </div>
</template>
<style scoped>
.container {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
}
.video-container {
  position: relative;
  width: 100%;
  justify-items: center;
  justify-content: center;
}

.remote-video {
  position: relative;
  top: 0;
  left: 0;
  object-fit: contain;
  background-color: rgb(0, 0, 0);
}

.floating-button-container {
  position: absolute;
  top: 10px;
  right: 10px;
  z-index: 2;
}
.capture-button-container {
  position: absolute;
  top: 60px;
  right: 10px;
  z-index: 2;
}
.shader-button-container {
  position: absolute;
  top: 110px;
  right: 10px;
  z-index: 2;
}
</style>
