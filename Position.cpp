#include "Position.hpp"


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