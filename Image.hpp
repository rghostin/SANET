#ifndef __IMAGE_HPP_
#define __IMAGE_HPP_

#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <array>
#include <unistd.h>
#include "loguru.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Position.hpp"
#include "utils_log.hpp"

template<typename T>
class Image final {};


template<typename T, unsigned int N>
class Image<T[N]> final {
private :
    bool _is_complete=false;
    const uint8_t _nodeID;
    const uint32_t _timestamp;
    std::mutex _mutex_is_complete;
    std::array<T, N> _content;
    std::array<bool, N/CHUNK_SIZE> _fillstate_array;

    void _set_is_complete_flag();

public :
    Image(ImageChunkPacket);
    Image(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(const Image&) = delete;
    Image& operator=(const Image&&) = delete;
    ~Image()=default;

    bool add_chunk(ImageChunkPacket);
    uint8_t get_nodeID();
    uint32_t get_timestamp();
};


template<typename T, unsigned int N>
Image<T[N]>::Image(ImageChunkPacket packet) : _nodeID(packet.nodeID), _timestamp(packet.timestamp), _mutex_is_complete(), \
        _content(), _fillstate_array() {
    this->add_chunk(packet);
}


template<typename T, unsigned int N>
bool Image<T[N]>::add_chunk(ImageChunkPacket packet) {
    unsigned int index(packet.offset/CHUNK_SIZE);

    if (index > N) {
        LOG_F(WARNING, "Packet offset exceeded size_array : %s", packet.repr().c_str());
    }
    else {
        if (not _fillstate_array[index]) {
            _content[index] = packet.chunk_content;
            _fillstate_array[index] = true;
            LOG_F(3, "Added chunk to image content : %s", packet.repr().c_str());
        }
        else {
            LOG_F(3, "Ignored chunk to image content : %s", packet.repr().c_str());
        }
    }
}


template<typename T, unsigned int N>
void Image<T[N]>::_set_is_complete_flag() {
    {
        std::lock_guard<std::mutex> lock(_mutex_is_complete);
        _is_complete = true;
    }
    LOG_F(3, "ALERT_IS_COMPLETE_FLAG set");
}


template<typename T, unsigned int N>
uint8_t Image<T[N]>::get_nodeID() {
    return _nodeID;
}


template<typename T, unsigned int N>
uint32_t Image<T[N]>::get_timestamp() {
    return _timestamp;
}


#endif