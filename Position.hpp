#ifndef __POSITION_HPP_
#define __POSITION_HPP_

#define POS_FMT "Pos(%f,%f)"
#define POS_REPR(p) (p).longitude, (p).latitude

struct Position {
    double longitude;
    double latitude;

    Position() : longitude(0), latitude(0) {}
    Position(double longitude, double latitude) : longitude(longitude), latitude(latitude) {}

    bool operator < (const Position &otherPos) const {
        if (longitude != otherPos.longitude) {
            if (longitude < otherPos.longitude) {
                return true;
            }
            else if (longitude > otherPos.longitude) {
                return false;
            }
        }
        else if (latitude != otherPos.latitude) {
            if (latitude < otherPos.latitude) {
                return true;
            }
            else if (latitude > otherPos.latitude) {
                return false;
            }
        }

        return false;
    }

    bool operator==(const Position& other) const {
        return longitude==other.longitude && latitude==other.latitude;
    }

    bool operator!=(const Position& other) const {
        return not(*this == other);
    }
};

#endif
