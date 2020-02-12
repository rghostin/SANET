#ifndef __IMAGEBUILDER_HPP_
#define __IMAGEBUILDER_HPP_

#include <mutex>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include "loguru.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Position.hpp"
#include "utils_log.hpp"
#include "Image.hpp"
#include "utils.hpp"
#include <openssl/md5.h>


class ImageBuilder final {
private :
    uint8_t _nodeID;
    uint32_t _timestamp;
    Position _position;
    uint32_t _sizeImage;
    uint32_t _sizeVec;
    bool _is_complete=false;
    mutable std::mutex _mutex_is_complete;
    Image _image;
    std::vector<std::array<char, IMG_CHUNK_SIZE>> _img_building_vec;
    std::vector<bool> _fillstate_vec;

public :
    ImageBuilder();
    explicit ImageBuilder(ImageChunkPacket);
    ImageBuilder(const ImageBuilder&)=delete;
    ImageBuilder(ImageBuilder&&) noexcept;
    ImageBuilder& operator=(const ImageBuilder&)=delete;
    ImageBuilder& operator=(const ImageBuilder&&)=delete;
    ~ImageBuilder()=default;

    void add_chunk(ImageChunkPacket);
    bool is_complete() const;
    Image get_image() const;
    uint32_t get_timestamp() const;
    uint32_t loss_percent() const;
};

#endif