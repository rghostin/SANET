from threading import Thread, Lock
from itertools import cycle
from utils import euclidian_distance
from time import sleep


POS_CFG_FILE = "current.position"

class Autopilot:
    def __init__(self, flightplan, speed):
        self.__speed = speed

        self.__mutex_position = Lock()
        self.__position = (None, None)

        self.__mutex_new_fp_flag = Lock()
        self.__new_fp_flag = False
        self.__flightplan = flightplan

        self.__delta_dist_wp = euclidian_distance(self.__flightplan.route[0], self.__flightplan.route[1])
        self.__delta_time_wp = self.__delta_dist_wp / self.__speed

        self.__thread_update_position = None
 
    @property
    def position(self):
        pos = None
        self.__mutex_position.acquire()
        try:
            pos = self.__position
        finally:
            self.__mutex_position.release()
        return pos

    @position.setter
    def position(self, newpos):
        self.__mutex_position.acquire()
        try:
            self.__position = newpos
        finally:
            self.__mutex_position.release()
        
        to_write = "%s, %s" % newpos
        with open(POS_CFG_FILE, 'r') as f:
            f.write(to_write)

    def __is_new_fp_flag(self):
        flag_val = None
        self.__mutex_new_fp_flag.acquire()
        try:
            flag_val = self.__new_fp_flag
        finally:
            self.__mutex_new_fp_flag.release()
        return flag_val

    def __reset_new_fp_flag(self): 
        self.__mutex_new_fp_flag.acquire()
        try:
            self.__new_fp_flag = False
        finally:
            self.__mutex_new_fp_flag.release()

    def __set_new_fp_flag(self): 
        self.__mutex_new_fp_flag.acquire()
        try:
            self.__new_fp_flag = True
        finally:
            self.__mutex_new_fp_flag.release()

    def __tr_update_position(self):
        while True:
            it = cycle(self.__flightplan.route)

            # seek start_waypoint
            while not it==self.__flightplan.start_waypoint:
                wp = next(it)    

            # todo: current version drone teleported to new pos -- to fix
            while True:
                if self.__is_new_fp_flag():
                    self.__reset_new_fp_flag()
                    break
                self.set_position = wp
                sleep(self.__delta_time_wp)
                wp = next(it)
    
    def notifyFligthPlan(self, flightplan):
        self.__flightplan = flightplan
        self.__set_new_fp_flag()

    def start(self):
        self.__thread_update_position = Thread(target=self.__tr_update_position)
        self.__thread_update_position.start()
 
    def join(self):
        self.__thread_update_position.join()
