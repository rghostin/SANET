#ifndef __IMAGE_HPP_
#define __IMAGE_HPP_

#include <mutex>
#include <vector>
#include <unistd.h>
#include "loguru.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Position.hpp"
#include "utils_log.hpp"


class Image final {
private :
    bool _is_complete=false;
    const uint8_t _nodeID;
    const uint32_t _timestamp;
    std::mutex _mutex_is_complete;
    std::vector<std::array<char, CHUNK_SIZE>> _content;
    std::vector<bool> _fillstate_array;

    void _set_is_complete_flag();

public :
    explicit Image(ImageChunkPacket);
    Image(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(const Image&) = delete;
    Image& operator=(const Image&&) = delete;
    ~Image()=default;

    bool add_chunk(ImageChunkPacket);
    uint8_t get_nodeID();
    uint32_t get_timestamp();
};


Image::Image(ImageChunkPacket packet) : _nodeID(packet.nodeID), _timestamp(packet.timestamp), _mutex_is_complete(), \
        _content(packet.sizeImage/CHUNK_SIZE), _fillstate_array(packet.sizeImage/CHUNK_SIZE) {
    this->add_chunk(packet);
}


bool Image::add_chunk(ImageChunkPacket packet) {
    unsigned int index(packet.offset/CHUNK_SIZE);

    if (index > _content.size()) {
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


void Image::_set_is_complete_flag() {
    {
        std::lock_guard<std::mutex> lock(_mutex_is_complete);
        _is_complete = true;
    }
    LOG_F(3, "ALERT_IS_COMPLETE_FLAG set");
}


uint8_t Image::get_nodeID() {
    return _nodeID;
}


uint32_t Image::get_timestamp() {
    return _timestamp;
}


#endif