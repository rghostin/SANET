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


inline void print_md5_sum(unsigned char* md) {
    char checksum_res[32];

    for(int i=0; i < MD5_DIGEST_LENGTH; i++) {
        std::snprintf(checksum_res + (i * 2), MD5_DIGEST_LENGTH, "%02x", md[i]);
    }
    LOG_F(3, "MD5 - Checksum : %s", checksum_res);
}


#endif