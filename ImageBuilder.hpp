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

#endif