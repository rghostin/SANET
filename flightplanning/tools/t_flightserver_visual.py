from flight_server import FlightServer


GLOBAL_AREA_POLYGON_PATH = "global_area.polygon"
SCOPE = 20
SERVER_PATH = "testc/usocket"

fp = FlightServer(GLOBAL_AREA_POLYGON_PATH, SCOPE, SERVER_PATH, display=True)
fp.start()