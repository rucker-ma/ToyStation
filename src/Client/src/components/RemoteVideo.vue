<script setup lang="ts">
import { ref } from "vue";
import { RtcClient } from "../transfer/rtc";
import {NButton,NLayout,NCard} from "naive-ui";

const client = new RtcClient();
const remoteVideo = ref<InstanceType<typeof HTMLVideoElement>>();

function connectToServer() {
  client.pc.ontrack = getTrack;
  client.connect();
}


function getTrack(ev: RTCTrackEvent): void {
  if (ev.track.kind === "video") {
    if (remoteVideo.value) {
      remoteVideo.value.srcObject = ev.streams[0];
    }
  }
}
let filter_key: string[] = ["w", "s", "a", "d"];

document.addEventListener("keydown", function (e) {
  if (filter_key.find((candidate) => candidate === e.key) !== undefined) {
    const keyinfo={
      type:'keydown',
      value:e.keyCode
    }
    let content = JSON.stringify(keyinfo)
    client.sendInput(content)    
  }
});
document.addEventListener("keyup",(e:KeyboardEvent)=>{
  if (filter_key.find((candidate) => candidate === e.key) !== undefined) {
    const keyinfo={
      type:'keyup',
      value:e.keyCode
    }
    let content = JSON.stringify(keyinfo)
    client.sendInput(content)    
  }
});
</script>

<template>
  <n-layout embedded content-style="padding: 20px;">
    <video id="remote-video" ref="remoteVideo" playsinline autoplay></video>
  </n-layout>
  <n-button @click="connectToServer" style="margin:20px;align-items: center;">Connect</n-button>
</template>

<style scoped>

video {
  object-fit: fill;
  width: 100%;
  height: 100%;
}
</style>
