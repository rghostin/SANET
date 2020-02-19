#include "ImageBuilder.hpp"


inline uint32_t compute_sizeVec(const ImageChunkPacket& packet) {
    double d = static_cast<double>(packet.sizeImage) / static_cast<double>(IMG_CHUNK_SIZE);
    return static_cast<uint32_t>(ceil(d));
}

inline uint32_t compute_indexVec(const ImageChunkPacket& packet) {
    double d = static_cast<double>(packet.offset) / static_cast<double>(IMG_CHUNK_SIZE);
    return static_cast<uint32_t>(ceil(d));
}

ImageBuilder::ImageBuilder() :_nodeID(0), _timestamp(0), _position(), _sizeImage(0), _sizeVec(0),
        _is_global(false), _mutex_is_complete(), _image(), _img_building_vec(), _fillstate_vec() {}


ImageBuilder::ImageBuilder(ImageChunkPacket packet) :
        _nodeID(packet.nodeID), _timestamp(packet.timestamp), _position(packet.position), _sizeImage(packet.sizeImage),
        _sizeVec(compute_sizeVec(packet)), _is_global(packet.is_global),
        _mutex_is_complete(), _image(), _img_building_vec(_sizeVec),
        _fillstate_vec(_sizeVec) {
    this->add_chunk(packet);
}


ImageBuilder::ImageBuilder(ImageBuilder&& other) noexcept : 
    _nodeID(other._nodeID), _timestamp(other._timestamp), _position(other._position),
    _sizeImage(other._sizeImage), _sizeVec(other._sizeVec),_is_complete(other._is_complete),
    _is_global(other._is_global), _mutex_is_complete(), _image(std::move(other._image)),
    _img_building_vec(std::move(other._img_building_vec)), _fillstate_vec(std::move(other._fillstate_vec)) 
        {}


void ImageBuilder::add_chunk(ImageChunkPacket packet) {
    {
        std::lock_guard<std::mutex> lock(_mutex_is_complete);
        if (_is_complete) return;
    }

    auto index(compute_indexVec(packet));


    if (not _fillstate_vec[index]) {
        _img_building_vec[index] = packet.chunk_content;
        _fillstate_vec[index] = true;
        LOG_F(3, "Added chunk to img_building_vec : %s", packet.repr().c_str());

        // if was last chunk, build Image struct and calc MD5
        if ( std::all_of(_fillstate_vec.begin(), _fillstate_vec.end(), [](bool b){return b;}) ){
            std::array<char, IMG_CHUNK_SIZE>::iterator it;
            uint32_t size_remaining(_sizeImage);
            int chunk_treated(0);

            {
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
            }
            LOG_F(WARNING, "Image (%d,%d) is complete- MD5: %s", _nodeID, _timestamp, get_md5_string(&_image.content[0], _image.content.size()).c_str());
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


uint8_t ImageBuilder::get_nodeid() const {
    return _nodeID;
}


bool ImageBuilder::is_global() const {
    return _is_global;
}


uint32_t ImageBuilder::get_loss_percent() const {
    long int counterLoss = std::count_if(_fillstate_vec.begin(), _fillstate_vec.end(), [](bool b){return b;});
    float res((static_cast<float>(counterLoss) / static_cast<float>(_sizeVec)) * 100);
    return static_cast<uint32_t>(res);
}


bool ImageBuilder::is_chunk_already_received(const ImageChunkPacket& packet) const {
    auto index(compute_indexVec(packet));
    
    return _fillstate_vec[index];
}