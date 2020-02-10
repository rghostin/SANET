#include "ImageBuilder.hpp"

ImageBuilder::ImageBuilder(ImageChunkPacket packet) :
        _nodeID(packet.nodeID), _timestamp(packet.timestamp), _position(packet.position), _sizeImage(packet.sizeImage),
        _mutex_is_complete(), _image(), _img_building_vec(packet.sizeImage/IMG_CHUNK_SIZE),
        _fillstate_array(packet.sizeImage/IMG_CHUNK_SIZE) {
    this->add_chunk(packet);
}


//ImageBuilder &ImageBuilder::operator=(const ImageBuilder &&imageBuilderCopy) {
//    if (this != &imageBuilderCopy) {
//        _nodeID = imageBuilderCopy._nodeID;
//        _timestamp = imageBuilderCopy._timestamp;
//        _position = imageBuilderCopy._position;
//        _sizeImage = imageBuilderCopy._sizeImage;
//        {
//            std::lock_guard<std::mutex> lock(imageBuilderCopy._mutex_is_complete);
//            _is_complete = imageBuilderCopy._is_complete;
//        }
//        _mutex_is_complete;
//        _image = imageBuilderCopy._image;
//        _img_building_vec = imageBuilderCopy._img_building_vec;
//        _fillstate_array = imageBuilderCopy._fillstate_array;
//    }
//    return *this;
//}


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

            int chunk_treated(0);

            for (auto & i : _img_building_vec) {
                (_image.content).insert(_image.content.begin() + (chunk_treated * IMG_CHUNK_SIZE), std::begin(i), std::end(i));
                chunk_treated += 1;
            }
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

