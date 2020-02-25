#ifndef CAMERA_CAMERA_HPP
#define CAMERA_CAMERA_HPP

#include <string>
#include <iostream>
#include "Position.hpp"
#include <opencv2/opencv.hpp>

class Camera final{
private:
    // global_image
    int _scope;
    cv::Mat _global_area;


public:
    Camera(std::string, int&);
    Camera(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera&operator=(const Camera&&) = delete;
    ~Camera()= default;

    void take_photo(Position, std::string);


};

#endif //CAMERA_CAMERA_HPP
