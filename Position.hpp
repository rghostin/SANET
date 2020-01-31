#ifndef __POSITION_HPP_
#define __POSITION_HPP_

#define POS_FMT "Pos(%f,%f)"
#define POS_REPR(p) (p).longitude, (p).latitude

struct Position {
    double longitude;
    double latitude;
};

#endif
