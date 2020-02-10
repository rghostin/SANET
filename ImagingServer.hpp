#ifndef __IMAGINGSERVER_HPP_
#define __IMAGINGSERVER_HPP_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <thread>
#include <mutex>
#include <map>
#include <queue>
#include "loguru.hpp"
#include "settings.hpp"
#include "AbstractReliableBroadcast.hpp"
#include "common.hpp"
#include "packets.hpp"
#include "Image.hpp"


class ImagingServer : public AbstractReliableBroadcastNode<ImageChunkPacket> {
private:
    // self information and settings
    std::queue<ImageChunkPacket> _chunkpacketqueue;
    unsigned short _image_reception_timeout;
    std::map<uint8_t, Image> _image_map;
    std::map<uint8_t, Image> _construction_image_map;

    // delegations
    std::thread _thread_check_img_in_construct;
    std::mutex _mutex_check_img_in_construct;
    std::mutex _mutex_img_map;

    ImageChunkPacket _produce_packet() override;
    void _process_packet(const ImageChunkPacket&) override;
    void _tr_check_img_in_construct_map();

public:
    ImagingServer(unsigned short port, uint8_t nodeID, unsigned short image_reception_timeout);
    ImagingServer(const ImagingServer&) = delete;
    ImagingServer(ImagingServer&&) = delete;
    ImagingServer& operator=(const ImagingServer&) = delete;
    ImagingServer& operator=(const ImagingServer&&) = delete;
    ~ImagingServer() override;

    void start() override;
    void join() override;

};


#endif