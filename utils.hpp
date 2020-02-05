#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <cstdio>

int read_int_from_file(const char* file_name){
  FILE* file = fopen (file_name, "r");
  int i = 0;
  fscanf (file, "%d", &i);     
  fclose (file);        
  return i;
}


#endif