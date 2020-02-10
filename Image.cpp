#include "Image.hpp"

Image::Image(uint8_t nodeID, uint32_t timestamp, unsigned int size) : nodeID(nodeID), timestamp(timestamp), size_array(size/CHUNK_SIZE), \
        content(nullptr), fillstate_array(nullptr) {
    if (size > CHUNK_SIZE) {
        content = new char*[size_array];
        fillstate_array = new bool[size_array];

        for (unsigned int i = 0; i < size_array; ++i) {
            content[i] = new char[CHUNK_SIZE];
        }
    }
}


Image::~Image() {
    if (content) {
        for (unsigned int i = 0; i < size_array; ++i) {
            delete[](content[i]);
        }

        delete(content);
        content = nullptr;
    }

    if (fillstate_array) {
        delete(fillstate_array);
        fillstate_array = nullptr;
    }
}


bool Image::add_chunk(ImageChunkPacket packet) {
    unsigned int index(packet.offset/CHUNK_SIZE);

    if (index > size_array) {
        LOG_F(WARNING, "Packet offset exceeded size_array : %s", packet.repr().c_str());
    }
    else {
        if (not fillstate_array[index]) {

        }



        LOG_F(3, "Added chunk to image content : %s", packet.repr().c_str());
    }
}


uint8_t Image::get_nodeID() {
    return nodeID;
}


uint32_t Image::get_timestamp() {
    return timestamp;
}