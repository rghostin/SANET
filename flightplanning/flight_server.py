import socket
import json
import os
from flight_planner import FlightPlanner
from utils import plotAllFlightPlans, parseNodeId
from autopilot import Autopilot
import global_settings as gs


class FlightServer:
    def __init__(self, polygon_path, scope, server_address, display=False):
        self.assertServerAddressNotUsed(server_address)
        self.__server_addr_ = server_address
        self.__usockfd_ = None
        self.__fplanner_ = FlightPlanner(polygon_path, scope)
        self.__display_ = display
        self.__connection_ = None
        self.__my_nodeId_ = parseNodeId(gs.NODE_ID_PATH)
        self.__autopilot = Autopilot(speed=gs.AUTOPILOT_SPEED, nodeID=self.__my_nodeId_)     # 1 waypoint / sec
        self.__last_status_node_map_ = dict()


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
        if not(status_node_map== self.__last_status_node_map_):
            self.__last_status_node_map_ = status_node_map


            if "255" in status_node_map:
                self.__fplanner_.notifyNewPoly()
                del status_node_map[255]


            self.__fplanner_.recompute(status_node_map)

            if self.__display_:
                plotAllFlightPlans(self.__fplanner_.flight_plans)
            
            for fplan in self.__fplanner_.flight_plans:
                if fplan.nodeid == self.__my_nodeId_:
                    print("Sending flight-plan to autopilot")
                    print(fplan)
                    self.__autopilot.set_flightplan(fplan)
                    break


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
        
        self.__autopilot.start()

        while True:
            self.__connection_, client_address = self.__usockfd_.accept()
            print('Connection open', client_address)

            received_chunk = self.__connection_.recv(4096)
            json_status_nodemap = received_chunk.decode() if received_chunk else None

            print("Received #", json_status_nodemap, "#")
            if not json_status_nodemap:
                print("closing connection")
                self.__connection_.close()  # Clean up the connection
                break
            self.processReceived(json_status_nodemap)
    
    def stop(self):
        self.__autopilot.stop()
        if self.__connection_:
            self.__connection_.close()  # Clean up the connection

if __name__ == "__main__":
    SCOPE = 43 # TODO camera

    fp = FlightServer(gs.GLOBAL_AREA_POLYGON_PATH, SCOPE, gs.USOCKET_PATH, display=True)
    try:
        fp.start()
    except KeyboardInterrupt:
        fp.stop()
