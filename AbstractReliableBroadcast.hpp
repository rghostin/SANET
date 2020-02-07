#ifndef _ABSTRACTRELIABLEBROADCASTNODE_HPP_
#define _ABSTRACTRELIABLEBROADCASTNODE_HPP_

#include <map>
#include <mutex>
#include "loguru.hpp"
#include "common.hpp"
#include "AbstractBroadcastNode.hpp"

#include "utils_log.hpp" //TODO rm



template<typename P>
class AbstractReliableBroadcastNode : public AbstractBroadcastNode<P> {
private:
    time_t _max_packet_age;
    mutable std::mutex _mutex_last_seq_map;
    std::map<uint8_t, std::pair<uint32_t, unsigned int>> _last_seq_map;

    //threads
    std::thread _thread_update_last_seq_map;
    void _tr_update_last_seq_map();

    // reliable overhead
    std::mutex _mutex_next_seqnum;
    uint32_t _next_seqnum=0;    

    bool _is_already_processed(const P&) const;
    virtual bool _to_be_ignored(const P&) const override;

protected:
    virtual void _process_packet(const P&) override;
    virtual P _produce_packet() override;

public:
    AbstractReliableBroadcastNode(uint8_t nodeID, unsigned short port, const char* name, time_t max_packet_age);
    virtual ~AbstractReliableBroadcastNode() = 0;   // forcing abstract

    virtual void start() override;
    virtual void join() override;
};


// implementation

template<typename P>
AbstractReliableBroadcastNode<P>::AbstractReliableBroadcastNode(uint8_t nodeID, unsigned short port, const char* name, time_t max_packet_age)  :
    AbstractBroadcastNode<P>(nodeID, port, name),
    _max_packet_age(max_packet_age) {}


template<typename P>
AbstractReliableBroadcastNode<P>::~AbstractReliableBroadcastNode() {
    if (_thread_update_last_seq_map.joinable()) {
        _thread_update_last_seq_map.join();
    }
}


template<typename P>
void AbstractReliableBroadcastNode<P>::_tr_update_last_seq_map() {
    loguru::set_thread_name(this->threadname("AgeSeqCheck").c_str());
    LOG_F(INFO,"Starting %s last_mapt_check", this->_name);
    while (! process_stop) {
        {
            std::lock_guard<std::mutex> lock(_mutex_last_seq_map);
            LOG_F(7, "last seq map:\n%s", print_log_map(_last_seq_map).c_str());
            for (auto it=_last_seq_map.begin(); it != _last_seq_map.end(); /*no increment*/ ) {
                unsigned int& age = (it->second).second;
                if (age <= 0) {
                    LOG_F(3, "Erased from _last_seq_map nodeID=%d, size=%lu", it->first, _last_seq_map.size()-1);
                    _last_seq_map.erase(it++);
                } else {
                    age--;
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOG_F(INFO, "process_stop=true; exiting");
}


template<typename P>
bool AbstractReliableBroadcastNode<P>::_is_already_processed(const P& packet) const {
    std::lock_guard<std::mutex> lock(_mutex_last_seq_map);
    const std::map<uint8_t, std::pair<uint32_t, unsigned int>>::const_iterator it = _last_seq_map.find(packet.nodeID);
    if (it != _last_seq_map.end()) {
        const uint32_t& seqnum = (it->second).first;
        const uint32_t& age = (it->second).second;
        return (packet.seqnum <= seqnum || age <= 0);
    }
    LOG_F(3, "Packet NOT already processed: %s", packet.repr().c_str());
    return false;
}


template<typename P>
inline bool AbstractReliableBroadcastNode<P>::_to_be_ignored(const P& packet) const {
    return AbstractBroadcastNode<P>::_to_be_ignored(packet) || _is_already_processed(packet);
}


template<typename P>
void AbstractReliableBroadcastNode<P>::_process_packet(const P& packet) {
    LOG_F(INFO, "Processing packet: %s", packet.repr().c_str());
    {
        std::lock_guard<std::mutex> lock(_mutex_last_seq_map);
        _last_seq_map[packet.nodeID] = std::pair<uint32_t, unsigned int>(packet.seqnum, _max_packet_age);
        LOG_F(3, "Inserted in _last_seq_map for nodeID=%d, size: %lu", packet.nodeID, _last_seq_map.size());
    }
    // in addition to the standard server, echo a broadcast of the packet for reliable broadcast
    LOG_F(3, "Echoing packet: %s", packet.repr().c_str());
    this->broadcast(packet);
}

template<typename P>
P AbstractReliableBroadcastNode<P>::_produce_packet() {
    P packet = AbstractBroadcastNode<P>::_produce_packet();
    {   
        std::lock_guard<std::mutex> lock(_mutex_next_seqnum);
        packet.seqnum = _next_seqnum++;
    }
    LOG_F(3, "Generated packet: %s", packet.repr().c_str());
    return packet;    
}

template<typename P>
void AbstractReliableBroadcastNode<P>::start() {
    AbstractBroadcastNode<P>::start();
    _thread_update_last_seq_map = std::thread(&AbstractReliableBroadcastNode::_tr_update_last_seq_map, this); 
}

template<typename P>
void AbstractReliableBroadcastNode<P>::join() {
    AbstractBroadcastNode<P>::join();
    _thread_update_last_seq_map.join();
}


#endif