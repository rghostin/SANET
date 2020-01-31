#include <map>
#include <mutex>
#include "MainServer.hpp"
#include "packets.hpp"

class ReliableMainServer : public MainServer {
private:
    std::mutex _mutex_last_seq_map;

protected:
    std::map<uint8_t, unsigned int> _last_seq_map;


    bool _is_already_processed(const Packet&) const;
    virtual bool _to_be_ignored(const Packet&) const override;
    virtual void _process_packet(const Packet&) const override;
};