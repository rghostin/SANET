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
#include <iostream>
#include "loguru.hpp"
#include "settings.hpp"
#include "AbstractBroadcastNode.hpp"
#include "common.hpp"
#include "settings.hpp"
#include "packets.hpp"
#include "ImageBuilder.hpp"
#include <cstdio>

#define INPUT_FILE(x) PATH_IMG #x TYPE_IMG


class ImagingServer : public AbstractBroadcastNode<ImageChunkPacket> {
private:
    // self information and settings
    uint32_t _image_reception_timeout=IMAGE_RECEPTION_TIMEOUT;
    uint32_t _image_reception_check=IMAGE_RECEPTION_CHECK;

    std::mutex _mutex_img_map;
    std::map<Position, Image> _image_map;
    std::mutex _mutex_check_img_in_construct;
    std::map<Position, ImageBuilder> _building_image_map;

    // threads
    std::thread _thread_check_completed_imgs;
    void _tr_check_completed_imgs();

    ImageChunkPacket _produce_packet() override;
    void _process_packet(const ImageChunkPacket&) override;
    void _send_image();
    bool _to_be_ignored(const ImageChunkPacket&) const override;

public:
    ImagingServer(unsigned short port, uint8_t nodeID);
    ImagingServer(const ImagingServer&) = delete;
    ImagingServer(ImagingServer&&) = delete;
    ImagingServer& operator=(const ImagingServer&) = delete;
    ImagingServer& operator=(const ImagingServer&&) = delete;
    ~ImagingServer() override;

    void start() override;
    void join() override;
};


#endif