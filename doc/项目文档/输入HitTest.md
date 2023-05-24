#### 方案一
在渲染时将object的id存入一个gbuffer中，鼠标点击时在gbuffer中读取鼠标点击点的值，验证是否选中对应的物体
方案二
屏幕空间逆变换后和boundingbox求交
