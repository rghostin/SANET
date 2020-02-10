#ifndef __IMAGEBUILDER_HPP_
#define __IMAGEBUILDER_HPP_

#include <mutex>
#include <vector>
#include <unistd.h>
#include <algorithm>
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
    Position _position;
    uint32_t _sizeImage;
    bool _is_complete=false;
    mutable std::mutex _mutex_is_complete;
    Image _image;
    std::vector<std::array<char, IMG_CHUNK_SIZE>> _img_building_vec;
    std::vector<bool> _fillstate_array;

public :
    explicit ImageBuilder(ImageChunkPacket);
    ImageBuilder(const ImageBuilder&) = delete;
    ImageBuilder(ImageBuilder&&) = delete;
    ImageBuilder& operator=(const ImageBuilder&) = delete;
    ImageBuilder& operator=(const ImageBuilder&&) = delete;
    ~ImageBuilder()=default;

    void add_chunk(ImageChunkPacket);
    bool is_complete() const;
    Image get_image() const;
};


ImageBuilder::ImageBuilder(ImageChunkPacket packet) :
_nodeID(packet.nodeID), _timestamp(packet.timestamp), _position(packet.position), _sizeImage(packet.sizeImage),
_mutex_is_complete(), _image(), _img_building_vec(packet.sizeImage/IMG_CHUNK_SIZE),
 _fillstate_array(packet.sizeImage/IMG_CHUNK_SIZE) {
    this->add_chunk(packet);
}


void ImageBuilder::add_chunk(ImageChunkPacket packet) {
    {
        std::lock_guard<std::mutex> lock(_mutex_is_complete);
        if (_is_complete) return;
    }

    uint32_t index(packet.offset/IMG_CHUNK_SIZE);

    if (not _fillstate_array[index]) {
        _img_building_vec[index] = packet.chunk_content;
        _fillstate_array[index] = true;
        LOG_F(3, "Added chunk to img_building_vec : %s", packet.repr().c_str());

        if ( std::all_of(_fillstate_array.begin(), _fillstate_array.end(), [](bool b){return b;}) ){
            std::lock_guard<std::mutex> lock(_mutex_is_complete);
            _is_complete = true;

            _image.nodeID = _nodeID;
            _image.timestamp = _timestamp;
            _image.position = _position;
            // _image.content TODO
        }
    }
    else {
        LOG_F(3, "Ignored chunk to img_building_vec : %s", packet.repr().c_str());
    }
}


bool ImageBuilder::is_complete() const {
    std::lock_guard<std::mutex> lock(_mutex_is_complete);
    return _is_complete;
}

Image ImageBuilder::get_image() const {
    std::lock_guard<std::mutex> lock(_mutex_is_complete);
    if (! _is_complete) {
        throw;
    }
    return _image;
}
#endif