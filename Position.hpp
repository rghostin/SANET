#ifndef POSITION_HPP
#define POSITION_HPP

class Position{
private :
    double _longitude;
    double _latitude;
public :
    Position();
    Position(double longitude, double latitude);
    double getLongitude();
    double getLatitude();
    ~Position()=default;
};
#endif //POSITION_HPP
