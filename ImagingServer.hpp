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
#include <functional>
#include "loguru.hpp"
#include "settings.hpp"
#include "AbstractBroadcastNode.hpp"
#include "common.hpp"
#include "ImageChunkPacket.hpp"
#include "ImageBuilder.hpp"
#include "database_utils/DbUtils.hpp"

#define INPUT_FILE(x) PATH_IMG #x TYPE_IMG  // TODO rm


class ImagingServer : public AbstractBroadcastNode<ImageChunkPacket> {
private:
    // self information and settings
    const uint32_t _image_reception_timeout=IMAGE_RECEPTION_TIMEOUT;
    const uint32_t _image_reception_check=IMAGE_TIMEOUT_CHECK_PERIOD;
    const uint32_t  _new_complete_image_check=IMAGE_CHECK_NEW_IMAGE_SLEEP_SEPARATOR;

    mutable std::mutex _mutex_image_map;
    std::map<Position, Image> _image_map;
    mutable std::mutex _mutex_building_image_map;
    std::map<Position, ImageBuilder> _building_image_map;

    // threads
    std::thread _thread_check_timeout_imgs;
    void _tr_check_timeout_imgs();
    std::thread _thread_check_new_complete_img;
    void _tr_check_new_global_img();
    bool has_global_img_changed();

    ImageChunkPacket _produce_packet() override;
    void _process_packet(const ImageChunkPacket&) override;
    void _send_image(bool);
    bool _to_be_ignored(const ImageChunkPacket&) const override;

public:
    ImagingServer(unsigned short port, uint8_t nodeID);
    ImagingServer(const ImagingServer&) = delete;
    ImagingServer(ImagingServer&&) = delete;
    ImagingServer& operator=(const ImagingServer&) = delete;
    ImagingServer& operator=(const ImagingServer&&) = delete;
    ~ImagingServer();

    void start() override;
    void join() override;
};


#endif