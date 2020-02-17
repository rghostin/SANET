from threading import Thread, Lock
from itertools import cycle
from utils import euclidian_distance, print_red
from time import sleep
import os
import global_settings as gs
from db_utils import db_update_currPos
import sqlite3


class Autopilot:
    def __init__(self, speed, nodeID):
        self.__speed = speed        # wp/sec
        self.__nodeID = nodeID

        self.__position = (None, None)

        self.__mutex_fp = Lock()
        self.__new_fp_flag = False
        self.__flightplan = None
        self.it = None
        self.next_wp = None

        self.__thread_update_position = None
        self.__mutex_halt = Lock()
        self.__halt = False

        self.__con = None

 

    @property
    def halt(self):
        val = None
        self.__mutex_halt.acquire()
        try:
            val = self.__halt
        finally:
            self.__mutex_halt.release()
        return val

    def set_halt(self):
        self.__mutex_halt.acquire()
        try:
            self.__halt = True
        finally:
            self.__mutex_halt.release()

    def set_position(self, newpos):
        self.__position = newpos 
        self.__con = sqlite3.connect(gs.DB_PATH)
        db_update_currPos(self.__con, self.__nodeID, longitude=newpos[0], latitude=newpos[1])
        self.__con.close()
        print_red("Position: %s" % str(newpos))

    def start(self):
        print_red("Starting AP")
        self.__thread_update_position = Thread(target=self.__tr_update_position)
        self.__thread_update_position.start()
 
    def stop(self):
        self.set_halt()
        self.__thread_update_position.join()
        print("Autopilot exiting")

    def set_flightplan(self, newfp):
        self.__mutex_fp.acquire()
        try:
            self.__flightplan = newfp
            self.__new_fp_flag = True
            self.it = cycle(self.__flightplan.route)
            while not self.next_wp==self.__flightplan.start_waypoint:
                self.next_wp = next(self.it) 
        finally:
            self.__mutex_fp.release()


    def __tr_update_position(self):
        print_red("starting thread")
        # todo: current version drone teleported to new pos -- to fix
        while not self.halt:
            self.__mutex_fp.acquire()
            try:
                if (self.__flightplan):
                    self.set_position(self.next_wp)
                    self.next_wp = next(self.it)
            finally:
                self.__mutex_fp.release()
            sleep(1 / self.__speed)
            
    

