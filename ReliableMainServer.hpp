#include <map>
#include <mutex>
#include "common.hpp"
#include "MainServer.hpp"
#include "packets.hpp"


class ReliableMainServer final : public MainServer {
private:
    time_t _max_packet_age;

    mutable std::mutex _mutex_last_seq_map;
    std::map<uint8_t, std::pair<uint32_t, unsigned int>> _last_seq_map;

    std::thread _thread_update_last_seq_map;
    void _tr_update_last_seq_map();

    bool _is_already_processed(const Packet&) const;
    virtual bool _to_be_ignored(const Packet&) const override;
    virtual void _process_packet(const Packet&) override;

public:
    ReliableMainServer(unsigned short port, uint8_t nodeID, Tracker &tracker, unsigned short heartTimer, time_t max_packet_age);
    virtual ~ReliableMainServer();
    virtual void start() override;
    virtual void join() override;
};