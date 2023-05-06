<script setup lang="ts">
import RemoteVideo from "../components/RemoteVideo.vue";
import Layout from "../components/Layout.vue";
</script>

<template>
  <main>
    <div class="dynamic-layout full-screen">
      <Layout />
    </div>
  </main>
</template>

<style scoped>
.dynamic-layout {
  padding: 2px;
}

.full-screen {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  z-index: 9999;
}
</style>
