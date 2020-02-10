#ifndef __IMAGE_HPP_
#define __IMAGE_HPP_

#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <unistd.h>
#include "loguru.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Position.hpp"
#include "utils_log.hpp"


class Image final {
private :
    bool is_complete=false;  // TODO : Redondant vu que bool add_chunk ?
    const uint8_t nodeID;
    const uint32_t timestamp;
    unsigned int size_array;
    char** content;
    bool* fillstate_array;

public :
    Image(uint8_t, uint32_t, unsigned int);
    Image(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(const Image&) = delete;
    Image& operator=(const Image&&) = delete;
    ~Image();

    bool add_chunk(ImageChunkPacket);
    uint8_t get_nodeID();
    uint32_t get_timestamp();
};


#endif //TRACKER_IMAGE_HPP
