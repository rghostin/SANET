import numpy as np
import socket
import json

class CCClient:
    CCCommands = {
        "FETCHALLNODES":2       # TODO: parse CCCommands
    }

    def __init__(self, ip_addr, port):
        self.__socket_ = None
        self.__ip_addr_ = ip_addr
        self.__port_ = port



    def __setUpSocket_(self):
        if self.__socket_ is not None:
            raise Exception("socket already connected")
        self.__socket_ = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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
        self._send_uint8(self.CCCommands["FETCHALLNODES"])
        status_node_map = self._receive_json()
        print(status_node_map)
        return status_node_map

    def start(self):
        self.__setUpSocket_()

    def stop(self):
        if self.__socket_ is not None:
            self.__socket_.close()
            print("Closing CCClient")

 

if __name__ == "__main__":
    cc_port = 6280
    client = CCClient('10.93.210.132', cc_port)
    client.start()
    client.stop()