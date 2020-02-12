#include "ImageBuilder.hpp"

ImageBuilder::ImageBuilder() :_nodeID(0), _timestamp(0), _position(), _sizeImage(0), _sizeVec(0),
        _mutex_is_complete(), _image(), _img_building_vec(), _fillstate_vec() {}


ImageBuilder::ImageBuilder(ImageChunkPacket packet) :
        _nodeID(packet.nodeID), _timestamp(packet.timestamp), _position(packet.position), _sizeImage(packet.sizeImage),
        _sizeVec(static_cast<uint32_t>(ceil(static_cast<float>(packet.sizeImage) / static_cast<float>(IMG_CHUNK_SIZE)))),
        _mutex_is_complete(), _image(), _img_building_vec(_sizeVec),
        _fillstate_vec(_sizeVec) {
    this->add_chunk(packet);
}


ImageBuilder::ImageBuilder(ImageBuilder&& other) noexcept : _nodeID(0), _timestamp(0), _position(), _sizeImage(0), _sizeVec(0),
        _mutex_is_complete(), _image(), _img_building_vec(), _fillstate_vec() 
        {
            _nodeID = other._nodeID;
            _timestamp = other._timestamp;
            _position = other._position;
            _sizeImage = other._sizeImage;
            _sizeVec = other._sizeVec;
            _is_complete = other._is_complete;
            // _mutex already initialized
            _image = std::move(other._image);
            _img_building_vec = std::move(other._img_building_vec);
            _fillstate_vec = std::move(other._fillstate_vec);

        }


void ImageBuilder::add_chunk(ImageChunkPacket packet) {
    {
        std::lock_guard<std::mutex> lock(_mutex_is_complete);
        if (_is_complete) return;
    }

    auto index(static_cast<uint32_t>(ceil(static_cast<float>(packet.offset) / static_cast<float>(IMG_CHUNK_SIZE))));

    if (not _fillstate_vec[index]) {
        _img_building_vec[index] = packet.chunk_content;
        _fillstate_vec[index] = true;
        LOG_F(3, "Added chunk to img_building_vec : %s", packet.repr().c_str());

        if ( std::all_of(_fillstate_vec.begin(), _fillstate_vec.end(), [](bool b){return b;}) ){
            std::array<char, IMG_CHUNK_SIZE>::iterator it;
            uint32_t size_remaining(_sizeImage);
            int chunk_treated(0);

            std::lock_guard<std::mutex> lock(_mutex_is_complete);
            _is_complete = true;

            _image.nodeID = _nodeID;
            _image.timestamp = _timestamp;
            _image.position = _position;


            for (auto & i : _img_building_vec) {
                if (size_remaining >= IMG_CHUNK_SIZE) {
                    size_remaining -= IMG_CHUNK_SIZE;
                    it = std::end(i);
                }
                else {
                    it = std::begin(i) + size_remaining;

                }
                (_image.content).insert(_image.content.begin() + (chunk_treated * IMG_CHUNK_SIZE), std::begin(i), it);
                chunk_treated += 1;
            }

            auto *md5_checksum = new unsigned char[MD5_DIGEST_LENGTH];  // TODO VOIR MD5_DIGEST_LENGTH vu que plus d'import
            MD5((unsigned char*)&_image.content[0], _image.content.size(), md5_checksum);
            print_md5_sum(md5_checksum);

            LOG_F(3, "Image is complete from : %s", packet.repr().c_str());
            delete[](md5_checksum);
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


uint32_t ImageBuilder::get_timestamp() const {
    return _timestamp;
}


uint32_t ImageBuilder::loss_percent() const {
    uint32_t counterFalse(0);

    for (auto && i : _fillstate_vec) {
        if (not i) {
            counterFalse += 1;
        }
    }

    float res((static_cast<float>(counterFalse) / static_cast<float>(_sizeVec)) * 100);
    LOG_F(WARNING, "RESULT FLOAT : %.6f", res);

    return static_cast<uint32_t>(res);
}