import os

CURR_DIR = os.path.dirname(__file__)
CONF_DIR = os.path.join(CURR_DIR, "../conf")


GLOBAL_AREA_POLYGON_PATH = os.path.join(CONF_DIR, "global_area.polygon")
USOCKET_PATH = os.path.join(CONF_DIR, "usocket")
FP_CURR_POS_FILE_PATH = os.path.join(CONF_DIR, "current.position")
FP_CURR_POS_LOCK_PATH = os.path.join(CONF_DIR, "current.position.lock")
NODE_ID_PATH = os.path.join(CONF_DIR, "nodeid.conf")
