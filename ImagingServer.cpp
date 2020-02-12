#include "ImagingServer.hpp"

ImagingServer::ImagingServer(unsigned short port, uint8_t nodeID) :
        AbstractReliableBroadcastNode<ImageChunkPacket>(nodeID, port, "ImgSrv", RELBC_PACKET_MAX_AGE),
        _mutex_img_map(), _image_map(), _mutex_check_img_in_construct(), _building_image_map(), _thread_check_completed_imgs() {}


ImagingServer::~ImagingServer() {
    if (_thread_check_completed_imgs.joinable()) {
        _thread_check_completed_imgs.join();
    }
}


void ImagingServer::_tr_check_completed_imgs() {
    uint32_t actualTimestamp;

    loguru::set_thread_name("ImagingSrv");
    LOG_F(INFO, "Starting ImagingSrv _check_completed_imgs");

    std::map<uint8_t, ImageBuilder>::const_iterator it;

    while (! process_stop) {
        {  // Scope spécifique sinon lock_guard va dans le sleep
            std::lock_guard<std::mutex> lock_building(_mutex_check_img_in_construct);

            for (it = _building_image_map.cbegin(); it != _building_image_map.cend(); /*no increment*/ ) {
                actualTimestamp = static_cast<uint32_t>(std::time(nullptr));

                if ((actualTimestamp - ((it->second).get_timestamp())) > _image_reception_timeout) {
                    // No response
                    LOG_F(WARNING, "Image lost from NodeID=%d, Lost rate : %d%%", it->first, (it->second).loss_percent());
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
    ImageChunkPacket packet = AbstractReliableBroadcastNode<ImageChunkPacket>::_produce_packet();
    packet.timestamp = static_cast<uint32_t>(std::time(nullptr));
    // TODO fournir donnée d'image -> char[250] + sizeImage + offset
    LOG_F(3, "Generated packet: %s", packet.repr().c_str());
    return packet;
}


void ImagingServer::_process_packet(const ImageChunkPacket& packet) {
    AbstractReliableBroadcastNode<ImageChunkPacket>::_process_packet(packet);

    std::lock_guard<std::mutex> lock_building(_mutex_check_img_in_construct);

    if (_building_image_map.find(packet.nodeID) == _building_image_map.end()) {  // New Image
        _building_image_map.emplace(packet.nodeID, ImageBuilder(packet));
    }
    else {
        _building_image_map[packet.nodeID].add_chunk(packet);
        if (_building_image_map[packet.nodeID].is_complete()) {
            {
                std::lock_guard<std::mutex> lock(_mutex_img_map);
                _image_map[packet.nodeID] = _building_image_map[packet.nodeID].get_image();
            }
            LOG_F(3, "Image moved to _image_map: %s", packet.repr().c_str());

            _building_image_map.erase(packet.nodeID);  // image completed -> delete ImageBuilder

            LOG_F(3, "Image deleted from _building_image_map : %s", packet.repr().c_str());
        }
    }
}


void ImagingServer::start() {
    LOG_F(WARNING, "Starting Imaging server");
    AbstractReliableBroadcastNode<ImageChunkPacket>::start();
    _thread_check_completed_imgs = std::thread(&ImagingServer::_tr_check_completed_imgs, this);
}


void ImagingServer::join() {
    AbstractReliableBroadcastNode<ImageChunkPacket>::join();
    _thread_check_completed_imgs.join();
    LOG_F(WARNING, "ImgServer: joined all threads");
}