#include "ImagingServer.hpp"

ImagingServer::ImagingServer(unsigned short port, uint8_t nodeID) :
        AbstractBroadcastNode<ImageChunkPacket>(nodeID, port, "ImgSrv"),
        _mutex_image_map(), _image_map(), _mutex_building_image_map(), _building_image_map(), _thread_check_timeout_imgs() {}


ImagingServer::~ImagingServer() {
    if (_thread_check_timeout_imgs.joinable()) {
        _thread_check_timeout_imgs.join();
    }
}


void ImagingServer::_tr_check_timeout_imgs() {
    uint32_t actualTimestamp;

    loguru::set_thread_name(threadname("checkImgTO").c_str());
    LOG_F(INFO, "Starting ImagingSrv _check_timeout_imgs");

    std::map<Position, ImageBuilder>::const_iterator it;

    while (! process_stop) {
        {  // Scope spécifique sinon lock_guard va dans le sleep
            std::lock_guard<std::mutex> lock_building(_mutex_building_image_map);

            for (it = _building_image_map.cbegin(); it != _building_image_map.cend(); /*no increment*/ ) {
                actualTimestamp = static_cast<uint32_t>(std::time(nullptr));

                if ((actualTimestamp - ((it->second).get_timestamp())) > _image_reception_timeout) {
                    // No response
                    LOG_F(WARNING, "Image lost from NodeID=%d, Lost rate : %d%%", (it->second).get_nodeid(), (it->second).get_loss_percent());
                    _building_image_map.erase(it++);
                }
                else {
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(_image_reception_check));
    }
    LOG_F(INFO, "ImagingSrv _check_completed_img - process_stop=true; exiting");
}


ImageChunkPacket ImagingServer::_produce_packet() {
    ImageChunkPacket packet = AbstractBroadcastNode<ImageChunkPacket>::_produce_packet();
    packet.timestamp = static_cast<uint32_t>(std::time(nullptr));  // TODO
    LOG_F(3, "Generated packet: %s", packet.repr().c_str());  // TODO remove vu qu'offset pas fixé ici mais spam (3 -> 7) ?
    return packet;
}


void ImagingServer::_process_packet(const ImageChunkPacket& packet) {
    std::lock_guard<std::mutex> lock_building(_mutex_building_image_map);

    if (_building_image_map.find(packet.position) == _building_image_map.end()) {  // New Image
        _building_image_map.emplace(packet.position, ImageBuilder(packet));
    }
    else {
        _building_image_map[packet.position].add_chunk(packet);
        if (_building_image_map[packet.position].is_complete()) {
            {
                std::lock_guard<std::mutex> lock(_mutex_image_map);
                _image_map[packet.position] = _building_image_map[packet.position].get_image();
            }
            LOG_F(3, "Image moved to _image_map: %s", packet.repr().c_str());

            _building_image_map.erase(packet.position);  // image completed -> delete ImageBuilder

            LOG_F(3, "Image deleted from _building_image_map : %s", packet.repr().c_str());
        }
    }
}


void ImagingServer::_send_image() {
    ImageChunkPacket packet = _produce_packet();
    uint32_t size_file_remaining;
    uint32_t bytes_to_treat(IMG_CHUNK_SIZE);

    // TODO rm -hardcoded 
    FILE* image_file;
    char path_img[](INPUT_FILE(_nodeID));

    memset(&packet.chunk_content, '\0', IMG_CHUNK_SIZE);
    packet.sizeImage = get_size(path_img);
    size_file_remaining = packet.sizeImage;
    image_file = fopen(path_img, "rb");

    if (IMG_CHUNK_SIZE > size_file_remaining) {
        bytes_to_treat = size_file_remaining;
    }
    else {
        bytes_to_treat = IMG_CHUNK_SIZE;
    }

    while(size_file_remaining > 0 and !feof(image_file)) {
        fread(&packet.chunk_content[0], sizeof(char), bytes_to_treat, image_file);

        this->broadcast(packet);

        LOG_F(7, "ImageChunkPacket: %s", packet.repr().c_str());

        size_file_remaining -= bytes_to_treat;
        packet.offset += bytes_to_treat;

        memset(&packet.chunk_content, '\0', IMG_CHUNK_SIZE);

        if (IMG_CHUNK_SIZE <= size_file_remaining) {
            bytes_to_treat = IMG_CHUNK_SIZE;
        }
        else {
            bytes_to_treat = size_file_remaining;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(IMAGE_SEND_SLEEP_SEPARATOR));
    }

    fclose(image_file);
}


void ImagingServer::start() {
    LOG_F(WARNING, "Starting Imaging server");
    AbstractBroadcastNode<ImageChunkPacket>::start();
    _thread_check_timeout_imgs = std::thread(&ImagingServer::_tr_check_timeout_imgs, this);
}


void ImagingServer::join() {
    AbstractBroadcastNode<ImageChunkPacket>::join();
    _thread_check_timeout_imgs.join();
    LOG_F(WARNING, "ImgServer: joined all threads");
}
 

inline bool ImagingServer::_to_be_ignored(const ImageChunkPacket& packet) const {
    bool chunk_already_received;
    bool image_already_constructed;

    {
        std::lock_guard<std::mutex> lock_building(_mutex_building_image_map);
        std::lock_guard<std::mutex> lock_constructed(_mutex_image_map);

        chunk_already_received = (_building_image_map.find(packet.position) != _building_image_map.end()) && (_building_image_map.at(packet.position).is_chunk_already_received(packet));
        image_already_constructed = _image_map.find(packet.position) != _image_map.end() && packet.timestamp == _image_map.at(packet.position).timestamp;
    }
    
    // ignore self NodeID || ignore packets for already received chunks || ignore packets for already build images
    return AbstractBroadcastNode<ImageChunkPacket>::_to_be_ignored(packet) || chunk_already_received || image_already_constructed;
} 