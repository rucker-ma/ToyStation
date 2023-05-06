<template>
  <div style="height: 100vh">
    <el-container style="height: 100%">
      <el-header class="main-header" style="height: 40px">Header</el-header>
      <el-container>
        <el-aside class="main-aside" style="width: 50px">Sid</el-aside>
        <el-main class="main-layout">
          <MainAreaLayout></MainAreaLayout>
        </el-main>
      </el-container>
      <el-footer class="main-footer">Footer</el-footer>
    </el-container>
  </div>
</template>

<script setup lang="ts">
// import RemoteVideo from "./RemoteVideo.vue";
import MainAreaLayout from "./MainAreaLayout.vue";
</script>
<style scoped>
.main-footer {
  height: 40px;
  width: 100%;
  /* position: fixed; 将footer固定在页面底部 */
  bottom: 0;
  background-color: #d12e2e;
}
.main-layout{
  min-height: calc(100vh - 40px - 40px);
  overflow: auto;
  background-color: gray;
  padding: 5px;
}
.main-aside{
  background-color: coral;
}
.main-header {
  background-color: aqua;
}
</style>
