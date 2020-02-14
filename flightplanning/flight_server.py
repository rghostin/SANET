import socket
import json
import os
from flight_planner import FlightPlanner
from utils import plotAllFlightPlans


class FlightServer:
    def __init__(self, polygon_path, scope, server_address, display=False):
        self.assertServerAddressNotUsed(server_address)
        self.__server_addr_ = server_address
        self.__usockfd_ = None
        self.__fplanner_ = FlightPlanner(polygon_path, scope)
        self.__display_ = display
        self.__connection_ = None

    @staticmethod
    def assertServerAddressNotUsed(server_address):
        # Make sure the socket does not already exist
        try:
            os.unlink(server_address)
        except OSError:
            if os.path.exists(server_address):
                raise

    def setup_usocket(self):
        self.__usockfd_ = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        # Bind the socket to the address
        print('starting up on {}'.format(self.__server_addr_))
        self.__usockfd_.bind(self.__server_addr_)
        self.__usockfd_.listen(1)  # Listen for incoming connections

    def processReceived(self, received_data):
        try:
            status_node_map = json.loads(received_data)
        except ValueError:
            print("Ignoring packet")
            return
        n = len(status_node_map)
        print(n, status_node_map)

        self.__fplanner_.recompute(status_node_map)
        if self.__display_:
            plotAllFlightPlans(self.__fplanner_.flight_plans)
        for fplan in self.__fplanner_.flight_plans:
            print("Sending flight-plan to autopilot")
            print(fplan.encoded_json)

    def receiveData(self, connection, client_address):
        # Receive the data in small chunks and retransmit it
        received_chunk = connection.recv(4096)
        if received_chunk:
            json_status_nodemap = received_chunk.decode()
            return json_status_nodemap
        else:
            print("Connection closed")
            return None

    def start(self):
        self.setup_usocket()
        print('Waiting for a connection')  # Wait for a connection
        self.__connection_, client_address = self.__usockfd_.accept()
        while True:
            print('Connection open', client_address)
            json_status_nodemap = self.receiveData(self.__connection_, client_address)
            if not json_status_nodemap:
                self.__connection_.close()  # Clean up the connection
                break
            self.processReceived(json_status_nodemap)
            print("processed first")
    
    def stop(self):
        self.__connection_.close()  # Clean up the connection

if __name__ == "__main__":
    CURR_DIR = os.path.dirname(__file__)

    SCOPE = 20
    GLOBAL_AREA_POLYGON_PATH = os.path.join(CURR_DIR, "global_area.polygon")
    USOCKET_PATH = os.path.join(CURR_DIR, "../usocket")

    fp = FlightServer(GLOBAL_AREA_POLYGON_PATH, SCOPE, USOCKET_PATH, display=False)
    try:
        fp.start()
    except KeyboardInterrupt:
        fp.stop()
