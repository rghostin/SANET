from threading import Thread, Lock
from itertools import cycle
from utils import euclidian_distance, print_red
from time import sleep
import os
import global_settings as gs

class Autopilot:
    def __init__(self, speed):
        self.__speed = speed        # wp/sec

        self.__position = (None, None)

        self.__mutex_fp = Lock()
        self.__new_fp_flag = False
        self.__flightplan = None
        self.it = None
        self.next_wp = None

        self.__thread_update_position = None
        self.__mutex_halt = Lock()
        self.__halt = False

 

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
        to_write = "%f\n%f" % newpos

        # Checking if file is locked (if FILENAME.LOCK exists)
        while os.access(gs.FP_CURR_POS_FILE_PATH, os.R_OK | os.X_OK):
            # file locked - wait
            pass

        try:
            # Locking the file
            with open(gs.FP_CURR_POS_LOCK_PATH, 'w'):
                print('creating lock')

            # Writing pos on file
            with open(gs.FP_CURR_POS_FILE_PATH, 'w') as pos_file:
                pos_file.write(to_write)
        finally:
            # Unlocking the file
            os.remove(gs.FP_CURR_POS_LOCK_PATH)
        print_red("Written %s to position" % str(newpos))

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
            
    

