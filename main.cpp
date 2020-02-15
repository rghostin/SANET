//
// Created by pahoran on 14.02.20.
//
#include "Position.hpp"
#include "Camera.hpp"

int main(int argc, char** argv) {
    int alpha = 30;
    Camera drone("/home/pahoran/test.png", alpha);
    Position pos(50,50);
    drone.take_photo(pos, "photo.png");
    return 0;
}
