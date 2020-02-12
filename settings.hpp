#ifndef _SETTINGS_HPP_
#define _SETTINGS_HPP_

// Global settings
#define CFG_NODEID "nodeid.cfg"

// Command&Control settings
#define CC_SERVER_PORT  6280
#define CC_MAX_CONNECTIONS 10
#define CC_GC_PERIOD    10

// Batman specific settings
#define BATMAN_IFACE "bat0"

// Tracking settings
#define TRACKING_SERVER_PORT 5820
#define TRACKING_HEARTBEAT_PERIOD 30
#define TRACKING_PEER_LOSS_TIMEOUT 4 * TRACKING_HEARTBEAT_PERIOD
#define TRACKING_PERIOD_CHECK_NODEMAP TRACKING_HEARTBEAT_PERIOD

// Reliable Broadcast settings
#define RELBC_PACKET_MAX_AGE 2 * TRACKING_HEARTBEAT_PERIOD // heartbeat < max_packet_age < lost_peer_timeout

// Flight planning settings
#define FP_USOCKET_PATH "./usocket"

#endif
