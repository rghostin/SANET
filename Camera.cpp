#include "Camera.hpp"

#include <iostream>

Camera::Camera(std::string picture_path, int& alpha)
{
    _global_area = cv::imread(picture_path, 1);
    _scope = (_global_area.rows>_global_area.cols) ?  _global_area.rows : _global_area.cols;
    _scope = _scope/(2*alpha);
    std::cout << _global_area.rows << " " << _global_area.cols << " "<<_scope << std::endl;

}

void Camera::take_photo( Position pos, std::string save_path) {
    // Create photo scope
    int x = ((pos.longitude - _scope) > 0) ? pos.longitude : 0;
    int y = ((pos.latitude - _scope) > 0) ? pos.latitude : 0;
    int width = 10*_scope, height=10*_scope;
    std::cout << x << " " << y << " "<< width << " " << height << std::endl;
    // take photo
    cv::Mat photo(_global_area, cv::Rect(x, y, width, height));
    // save photo
    cv::imwrite(save_path, photo);
}