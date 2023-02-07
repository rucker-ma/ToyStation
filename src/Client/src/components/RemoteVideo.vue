<script setup lang="ts">
import { ref } from "vue";
import { RtcClient } from "../transfer/rtc";

const client = new RtcClient();
const remoteVideo = ref<InstanceType<typeof HTMLVideoElement>>();

function connectToServer() {
  client.pc.ontrack = getTrack;
  client.connect();
}

function getTrack(ev: RTCTrackEvent): void {
  if (ev.track.kind === "video") {
    // videoSource = ev.streams[0]
    if (remoteVideo.value) {
      remoteVideo.value.srcObject = ev.streams[0];
    }
  }
}
</script>

<template>
  <video
    id="remote-video"
    ref="remoteVideo"
    playsinline
    autoplay
  ></video>
  <h2>Start</h2>
  <button @click="connectToServer">Connect</button>
</template>

<style scoped>
#remote-video {
  background-color: gray;
}
</style>
