#ifndef __POSITION_HPP_
#define __POSITION_HPP_

struct Position {
    double longitude;
    double latitude;
};


inline void print_position(const Position& pos) {
    printf("Longitude : %f\nLatitude: %f\n\n", pos.longitude, pos.latitude);
}

#endif
