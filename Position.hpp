#ifndef __POSITION_HPP_
#define __POSITION_HPP_

struct Position {
    double longitude;
    double latitude;
};


inline void print_position(const Position& pos) {
    printf("Longitude : %f\nLatitude: %f\n\n", pos.longitude, pos.latitude);
}


inline std::string get_positionInfos(const Position& pos) {
    return "Longitude : " + std::to_string(pos.longitude) + " | Lattitude: " + std::to_string(pos.latitude);
}


#endif
