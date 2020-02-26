#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <cstdio>
#include <cassert>
#include <vector>
#include <array>
#include <string>
#include <openssl/md5.h>
#include <fstream>
#include <cstring>
#include <sstream>
#include <algorithm>
#include "Position.hpp"

inline int read_int_from_file(const char* file_name){
  FILE* file;
  if ( (file = fopen (file_name, "r")) == NULL) {
      perror("Cannot open file");
      throw;
  }
  int i = 0;
  fscanf (file, "%d", &i);     
  fclose (file);        
  return i;
}

// TODO rm  hardcoded
inline uint32_t get_size(const char* file_name) {
    uint32_t res;

    FILE* file(fopen(file_name, "r"));
    fseek(file, 0L, SEEK_END);
    res = static_cast<uint32_t>(ftell(file));
    fclose(file);

    return res;
}


inline std::string get_md5_string(char* array, unsigned long size_array) {
    unsigned char md5_checksum[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<unsigned char *>(&array[0]), size_array, md5_checksum);

    char checksum_res[32];

    // converting hex to str
    for(int i=0; i < MD5_DIGEST_LENGTH; i++) {
        std::snprintf(checksum_res + (i * 2), MD5_DIGEST_LENGTH, "%02x", md5_checksum[i]);
    }

    return checksum_res;
}


inline std::vector<Position> read_global_poly(const char* global_poly_path) {
    std::ifstream ifs;             // creates stream ifs
    Position position;
    char comma;
    std::vector<Position> globalpoly;

    ifs.open(global_poly_path);  //opens file
    if (ifs.fail()) {
        perror("Cannot open file");
        throw;
    }

    while (true){
        ifs >> position.longitude >> comma >> position.latitude;
        if (ifs.fail() || ifs.eof()) {
            break;
        }
        globalpoly.push_back(position);
        position = Position();
    }
    ifs.close();
    return globalpoly;
}


template<size_t N>
inline std::array<char, N> string_to_chararray(const std::string& s) {
    assert (s.size() < N);
    std::array<char, N> arr;
    std::copy(s.begin(), s.end(), arr.data());
    arr[s.size()] = 0x00;
    return arr;
}

#endif