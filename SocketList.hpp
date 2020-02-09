#include <cstddef>
#include <vector>
#include "settings.hpp"
#include "loguru.hpp"


class SocketList final {
private:
    const size_t _max_size=CC_MAX_CONNECTIONS;
    size_t _len=0;
    int _connsockfds[CC_MAX_CONNECTIONS];
public:
    SocketList(size_t max_size) : _max_size(max_size) {
        for (unsigned i=0; i<_max_size; ++i) {
            _connsockfds[i] = 0;
        }
    }
    SocketList(const SocketList&) = delete;
    SocketList(SocketList&&) = delete;
    SocketList& operator=(const SocketList&) = delete;
    SocketList& operator=(const SocketList&&) = delete;
    ~SocketList() = default;

    size_t len() const { return _len;}

    void add_socket(int connsockfd) {
        if (_len >= _max_size) {
            throw "socketlist full";    // todo catch
        }
        for (unsigned i=0; i<_max_size; ++i) {
            if (_connsockfds[i] == 0) {
                _connsockfds[i] = connsockfd;
                ++_len;
                LOG_F(INFO, "Added socket %d to socketlist, len: %d", connsockfd, _len);
            }
        }
    }

    int get_max() const {
        int max_sd = 0;
        for (unsigned i=0; i<_max_size; ++i) {
            if (_connsockfds[i] > max_sd) {
                max_sd = _connsockfds[i];
            }
        }
        return max_sd;
    }

    // TODO make iterator
    std::vector<int> online_sockets() const {
        std::vector<int> sockets;
        for (unsigned i=0; i<_max_size; ++i) {
            if (_connsockfds[i] != 0) {
                sockets.push_back(_connsockfds[i]);
            }
        }
        return sockets;
    }
};