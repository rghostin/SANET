#ifndef _JSON_UTILS_HPP_
#define _JSON_UTILS_HPP_

#include "Position.hpp"


inline std::string json_getPolygon(std::vector<Position> globalpoly) {
    /* vector of vertices to json index:[x, y] */
    char buffer[1024]="";
    std::string res = "{";
    
    for (unsigned i=0; i<globalpoly.size(); ++i) {
        const Position& pos = globalpoly[i];

        snprintf(buffer, sizeof(buffer), "\"%u\": [%f, %f]", i, pos.longitude, pos.latitude);
        res += buffer;
        res += ",";
        memset(buffer, 0, sizeof(buffer));
    }
    res.pop_back();
    res += "}";
    return res;
}


inline void json_write_poly_to_file(const std::string& json_poly, const char* fname) {
    /*write .polygon file from json polygon */
    FILE* polygon_file(fopen(fname, "wb")); 

    char buffer[2048];
    memset(buffer, '\0', sizeof(buffer));

    std::stringstream ss(json_poly);
    std::string temp;

    for (int i = 0; i < std::count(json_poly.begin(), json_poly.end(), '['); ++i) {
        std::getline(ss, temp, '[');
        std::getline(ss, temp, ']');
        temp += "\n";
        fwrite(temp.c_str(), sizeof(char), temp.size(), polygon_file);
    }
    fclose(polygon_file);
}

#endif