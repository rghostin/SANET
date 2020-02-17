import numpy as np
import socket

def parseHppSetting(setting):
    setting_value = None
    with open("settings.hpp", "r") as settings_file:
        for line in settings_file:
            line = line.strip().split()
            if len(line) != 0 and line[1]==setting:
                setting_value = int(line[2])
                break
    return setting_value

class CCClient:
    def __init__(self, ip_addr, port):
        self.__socket_ = None
        self.__ip_addr_ = ip_addr
        self.__port_ = port

    def __setUpSocket_(self):
        self.__socket_ = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__socket_.connect((self.__ip_addr_, self.__port_))

    def send_uint8(self, to_send):
        print("Sending [uint8]", to_send)
        self.__socket_.send(np.uint8(to_send))


    def start(self):
        self.__setUpSocket_()
        while True:
            try:
                command_to_send = int(input("Input a command: "))
                self.send_uint8(command_to_send)
            except KeyboardInterrupt:
                self.stop()

    def stop(self):
        print("Closing CCClient")
        self.__socket_.close()


if __name__ == "__main__":
    cc_port = parseHppSetting("CC_SERVER_PORT")
    client = CCClient('localhost', cc_port)
    client.start()