#pragma once
namespace toystation {
// struct Vector2 {
//     int x;
//     int y;
// };
// struct Vector3 {
//     int x;
//     int y;
//     int z;
// };
// struct Vector4 {
//     int x;
//     int y;
//     int z;
//     int w;
// };

struct Size2d {
    Size2d() : width(0), height(0) {}
    Size2d(int inputw, int inputh) {
        width = inputw;
        height = inputh;
    }
    int width;
    int height;
};

}  // namespace toystation