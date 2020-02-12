#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <cstdio>
#include "loguru.hpp"
#include <openssl/md5.h>

inline int read_int_from_file(const char* file_name){
  FILE* file = fopen (file_name, "r");
  int i = 0;
  fscanf (file, "%d", &i);     
  fclose (file);        
  return i;
}


inline uint32_t get_size(const char* file_name) {
    uint32_t res;

    FILE* file(fopen(file_name, "r"));
    fseek(file, 0L, SEEK_END);
    res = static_cast<uint32_t>(ftell(file));
    fclose(file);

    return res;
}


inline void print_md5_sum(char* array, unsigned long size_array) {
    auto *md5_checksum = new unsigned char[MD5_DIGEST_LENGTH];
    {
        MD5(reinterpret_cast<unsigned char *>(&array[0]), size_array, md5_checksum);
        char checksum_res[32];

        for(int i=0; i < MD5_DIGEST_LENGTH; i++) {
            std::snprintf(checksum_res + (i * 2), MD5_DIGEST_LENGTH, "%02x", md5_checksum[i]);
        }
        LOG_F(3, "MD5 - Checksum : %s", checksum_res);
    }
}


#endif