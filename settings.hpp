#ifndef _SETTINGS_HPP_
#define _SETTINGS_HPP_

// Command&Control settings
#define CC_SERVER_PORT  6280
#define CC_MAX_CONNECTIONS 10
#define CC_GC_PERIOD    10

// Batman specific settings
#define BATMAN_IFACE "bat0"

// Tracking settings
#define TRACKING_SERVER_PORT 5820
#define TRACKING_PEER_LOSS_TIMEOUT 30
#define TRACKING_INITIAL_FP_SLEEP 1 // TRACKING_PEER_LOSS_TIMEOUT

// Imaging settings
#define IMAGING_SERVER_PORT 5821
#define IMG_CHUNK_SIZE 1024
#define IMAGE_RECEPTION_TIMEOUT 60
#define IMAGE_TIMEOUT_CHECK_PERIOD IMAGE_RECEPTION_TIMEOUT/2
#define IMAGE_SEND_SLEEP_SEPARATOR 10        // msec
#define PATH_IMG "img/"         // TODO rm
#define TYPE_IMG ".png"         // TODO rm

// Reliable Broadcast settings
#define RELBC_PACKET_MAX_AGE (TRACKING_PEER_LOSS_TIMEOUT/2) // heartbeat < max_packet_age < lost_peer_timeout

// Flight planning settings
#define CFG_DIR "conf/"
#define CFG_NODEID CFG_DIR "nodeid.conf"
#define FP_USOCKET_PATH "/tmp/usocket" //CFG_DIR "usocket"
#define FP_CURR_POS_FILE_PATH CFG_DIR "current.position"
#define FP_CURR_POS_LOCK_PATH CFG_DIR "current.position.lock"
#define FP_AUTOPILOT_SPEED 0.5

// Database settings
#define DB_PATH "database_utils/sanet.db"

#endif
