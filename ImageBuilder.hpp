#ifndef __IMAGEBUILDER_HPP_
#define __IMAGEBUILDER_HPP_

#include <mutex>
#include <vector>
#include <unistd.h>
#include "loguru.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Position.hpp"
#include "utils_log.hpp"
#include "Image.hpp"


class ImageBuilder final {
private :
    uint8_t _nodeID;
    uint32_t _timestamp;
    bool _is_complete=false;
    std::mutex _mutex_is_complete;
    Image _image;
    std::vector<bool> _fillstate_array;

public :
    explicit ImageBuilder(ImageChunkPacket);
    ImageBuilder(const ImageBuilder&) = delete;
    ImageBuilder(ImageBuilder&&) = delete;
    ImageBuilder& operator=(const ImageBuilder&) = delete;
    ImageBuilder& operator=(const ImageBuilder&&) = delete;
    ~ImageBuilder()=default;

    bool add_chunk(ImageChunkPacket);
};


ImageBuilder::ImageBuilder(ImageChunkPacket packet) : _nodeID(packet.nodeID), _timestamp(packet.timestamp), _mutex_is_complete(), \
        _content(packet.sizeImage/CHUNK_SIZE), _fillstate_array(packet.sizeImage/CHUNK_SIZE) {
    this->add_chunk(packet);
}


bool ImageBuilder::add_chunk(ImageChunkPacket packet) {
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


void ImageBuilder::_set_is_complete_flag() {
    {
        std::lock_guard<std::mutex> lock(_mutex_is_complete);
        _is_complete = true;
    }
    LOG_F(3, "ALERT_IS_COMPLETE_FLAG set");
}


uint8_t ImageBuilder::get_nodeID() {
    return _nodeID;
}


uint32_t ImageBuilder::get_timestamp() {
    return _timestamp;
}


#endif