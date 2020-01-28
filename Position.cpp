#include "Position.hpp"


Position::Position() {
    _longitude = 0;
    _latitude = 0;
}

Position::Position(double longitude, double latitude) {
    _longitude = longitude;
    _latitude = latitude;
}

double Position::getLongitude() {
    return _longitude;
}

double Position::getLatitude() {
    return _latitude;
}