import os

CURR_DIR = os.path.dirname(__file__)
CONF_DIR = os.path.join(CURR_DIR, "../conf")
DB_DIR = os.path.join(CURR_DIR, "../database_utils")

GLOBAL_AREA_POLYGON_PATH = os.path.join(CONF_DIR, "global_area.polygon")
USOCKET_PATH = "/tmp/usocket" #os.path.join(CONF_DIR, "usocket")
FP_CURR_POS_FILE_PATH = os.path.join(CONF_DIR, "current.position")
FP_CURR_POS_LOCK_PATH = os.path.join(CONF_DIR, "current.position.lock")
NODE_ID_PATH = os.path.join(CONF_DIR, "nodeid.conf")
AUTOPILOT_SPEED=2
DB_PATH=os.path.join(DB_DIR, "sanet.db")

ALPHA = 30

#GUI resources
RSRCIMG_DIR = os.path.join(CURR_DIR, "resource_images")
RSRCGUI_DIR = os.path.join(RSRCIMG_DIR, "gui")
CROPPEDIMG_DIR = os.path.join(RSRCIMG_DIR, "cropped_images")
IMGSET_DIR = os.path.join(RSRCIMG_DIR, "image_set")

MENU_WELCOME_PICTURE_PATH = os.path.join(RSRCGUI_DIR, "logo.jpg")
DRONE_PHOTO_PATH = os.path.join(RSRCGUI_DIR, "drone.png")

GLOBAL_AREA_IMG_PATH = os.path.join(CROPPEDIMG_DIR,"croped_image.jpg")
GLOBAL_AREA_IMG_PATH_BLACKMASK = os.path.join(CROPPEDIMG_DIR, "croped_image_black.png")
GLOBAL_AREA_IMG_PATH_FPS = os.path.join(CROPPEDIMG_DIR, "croped_image_fps.png")
GLOBAL_AREA_IMG_PATH_RECONSTRUCTION = os.path.join(CROPPEDIMG_DIR, "croped_image_reconstruction.png")

GLOBAL_AREA_IMAGE_TEST = os.path.join(IMGSET_DIR, "map_7.jpg")
