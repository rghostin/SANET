import json
import socket


import global_settings as gs
import numpy as np
from json_utils import json_positionsFromList
from utils import parsePolygonFile


def parseHppSettings(settings_path):
    lines = list()
    with open(settings_path, "r") as file:
        for line in file:
            stripped_splitted_line = line.strip().split()
            if len(stripped_splitted_line) > 2:
                lines.append(stripped_splitted_line)
    settings_map = {lines[i][1]: int(lines[i][2]) for i in range(len(lines))}
    return settings_map


class CCClient:
    CCCommands = parseHppSettings(gs.CCCOMMANDS_PATH)  

    def __init__(self, ip_addr, port):
        self.__socket_ = None
        self.__ip_addr_ = ip_addr
        self.__port_ = port



    def __setUpSocket_(self):
        if self.__socket_ is not None:
            raise Exception("socket already connected")
        self.__socket_ = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__socket_.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.__socket_.connect((self.__ip_addr_, self.__port_))

    def _send_uint8(self, to_send):
        if not 0<=to_send<256:
            raise TypeError("Uint8 out of bounds")
        print("Sending [uint8]=", to_send)
        self.__socket_.send(np.uint8(to_send))

    def _receive_json(self):
       received_chunk = self.__socket_.recv(4096)
       json_str = received_chunk.decode()
       return json.loads(json_str)

    def fetchAllNodes(self):
        self._send_uint8(self.CCCommands["FETCH_NODES_POS"])
        status_node_map = self._receive_json()
        print(status_node_map)
        return status_node_map

    def sendMapNumber(self, map_number):
        self._send_uint8(self.CCCommands["UPDATE_MAP_NUMBER"])
        self._send_uint8(map_number)

    def sendGlobalPolygon(self, global_area_polygon_path):
        global_polygon = parsePolygonFile(global_area_polygon_path)
        global_polyg_vertices = []
        for point in global_polygon.vertices:
            global_polyg_vertices.append((int(point[0]), int(point[1])))
        json_to_send = json_positionsFromList(global_polyg_vertices)
        print("Sending polygon:", json_to_send)
        self._send_uint8(self.CCCommands["UPDATE_GLOBAL_AREA_POLYGON"])
        self.__socket_.send(json_to_send.encode())                   # senidng json


    def fetchMapInfo(self):
        self._send_uint8(self.CCCommands["FETCH_NODES_POS"])
        global_area_polygon = self._receive_json()
        print(global_area_polygon)
        return global_area_polygon


    def fetchMapNumber(self):
        self._send_uint8(self.CCCommands["FETCH_MAP_NUMBER"])
        map_number = self.__socket_.recv(1024).decode()
        print(map_number)
        return int(map_number)


    def fetchGlobalPolygon(self):
        self._send_uint8(self.CCCommands["FETCH_GLOBAL_POLYGON"])
        global_area_polygon = self._receive_json()
        print(global_area_polygon)

        vertices_list = [None for _ in range(len(global_area_polygon))]
        for k, pos in global_area_polygon.items():
            vertices_list[int(k)] = tuple(pos)
        print(vertices_list)
        return vertices_list


    def start(self):
        self.__setUpSocket_()


    def stop(self):
        print("trying to close socket..")
        if self.__socket_ is not None:
            self.__socket_.shutdown(socket.SHUT_RDWR)
            self.__socket_.close()
            self.__socket_ = None
            print("Closing CCClient")

            
 

if __name__ == "__main__":
    cc_port = 6280
    client = CCClient('10.93.210.132', cc_port)
    client.start()
    client.stop()
