#include "geometry/rectangular_channel.hpp"

RectangularChannel::RectangularChannel(
    double length, 
    double width, 
    double height, 
    std::array<double, 3> center)
    :   length_(length),
        width_(width),
        height_(height),
        center_(center)
{}

double RectangularChannel::length() const {
    return length_;
}

double RectangularChannel::width() const {
    return width_;
}

double RectangularChannel::height() const {
    return height_;
}

std::array<double, 3> RectangularChannel::center() const {
    return center_;
}
