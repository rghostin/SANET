import sys
from map_gui import MapGUI
from PyQt5.QtWidgets import QWidget, QLabel, QApplication, QHBoxLayout, QVBoxLayout, QFrame
from PyQt5.QtWidgets import QPushButton, QDialog, QMessageBox
from PyQt5 import QtGui, QtCore
from time import sleep
import threading
from ccclient import CCClient
from copy import deepcopy

import global_settings as gs

WINDOW_TITLE = "CITY MAP"
CCLIENT_IP = "164.15.121.67"
CCLIENT_PORT = 6280

threadLock = threading.Lock()
threads = []


class UserGUI(QWidget):
    UPDATE_SLEEP_RATIO = 0.8

    def __init__(self):
        super().__init__()
        self.initUI()
        # Object MapGUI
        self.map_gui = MapGUI(window_name=WINDOW_TITLE,
                              points_filename=gs.GLOBAL_AREA_POLYGON_PATH,
                              crop_filename=gs.GLOBAL_AREA_IMG_PATH,
                              crop_withblack_filename=gs.GLOBAL_AREA_IMG_PATH_BLACKMASK,
                              crop_withfps_filename=gs.GLOBAL_AREA_IMG_PATH_FPS,
                              reconstruct_area_filename=gs.GLOBAL_AREA_IMG_PATH_RECONSTRUCTION,
                              drones_photo_path=gs.DRONE_PHOTO_PATH,
                              display=True)
        self.connected = False  # flag that shows connexion status
        self.chosen_map_path = None
        self.stop = False
        self.show_flight_plan = False
        self.N = None
        self.new_nodes = None

        # Select Area Window
        self.select_area_window = SelectMapWindow(self)
        self.select_area_window.setFixedSize(1000, 800)
        self.select_area_window.setModal(True)

        # Cclient objet
        self.cclient = CCClient(CCLIENT_IP, CCLIENT_PORT)

    def initUI(self):
        # Buttons

        # connect button
        self.connect_button = QPushButton("Connect")
        self.connect_button.setFixedSize(200, 50)
        self.connect_button.clicked.connect(self.connect_button_action)

        # select map button
        self.select_map_button = QPushButton("Select Map")
        self.select_map_button.setFixedSize(200, 50)
        self.select_map_button.clicked.connect(self.select_map_button_action)
        self.select_map_button.setDisabled(True)
        # select area button
        self.select_area_button = QPushButton("Select Area")
        self.select_area_button.setFixedSize(200, 50)
        self.select_area_button.clicked.connect(self.select_area_button_action)
        self.select_area_button.setDisabled(True)

        # calcul flight plans button
        self.send_area_button = QPushButton("Send AREA")
        self.send_area_button.setFixedSize(200, 50)
        self.send_area_button.clicked.connect(self.send_area_button_action)
        self.send_area_button.hide()

        # stop real time view button
        self.stop_button = QPushButton("STOP")
        self.stop_button.setFixedSize(200, 50)
        self.stop_button.clicked.connect(self.stop_button_action)
        self.stop_button.hide()

        # show/ hide flight plans button
        self.show_hide_button = QPushButton("Show/Hide Flight Plans")
        self.show_hide_button.setFixedSize(200, 50)
        self.show_hide_button.clicked.connect(self.show_hide_button_action)
        self.show_hide_button.hide()

        # real time button
        self.test_button = QPushButton("Execute Test")
        self.test_button.setFixedSize(200, 50)
        self.test_button.clicked.connect(self.start_test_button_action)
        self.test_button.hide()

        # Labels
        image = QtGui.QPixmap(gs.MENU_WELCOME_PICTURE_PATH)
        self.pic = QLabel(self)
        self.pic.setFixedSize(1200, 800)
        self.pic.setPixmap(image.scaled(self.pic.width(), self.pic.height(), QtCore.Qt.KeepAspectRatio))

        self.connect_led = QLabel(self)
        self.connect_led.setFixedSize(20, 20)
        image = QtGui.QPixmap(gs.RED_DOT_PATH)
        self.connect_led.setPixmap(
            (image.scaled(self.connect_led.width(), self.connect_led.height(), QtCore.Qt.KeepAspectRatio)))

        # Layouts

        division_line = QFrame()
        division_line.setFrameShape(QFrame.VLine)

        # Connect button layout
        hbox_connect_button = QHBoxLayout()
        hbox_connect_button.stretch(1)
        hbox_connect_button.addWidget(self.connect_button)
        hbox_connect_button.addWidget(self.connect_led)
        # buttons layout
        hbox_buttons = QHBoxLayout()
        hbox_buttons.addLayout(hbox_connect_button)
        hbox_buttons.addWidget(division_line)
        hbox_buttons.addWidget(self.select_map_button)
        hbox_buttons.addWidget(self.select_area_button)
        hbox_buttons.addWidget(self.send_area_button)
        hbox_buttons.addWidget(self.stop_button)
        hbox_buttons.addWidget(self.show_hide_button)
        hbox_buttons.addWidget(self.test_button)
        # picture layout
        self.hbox_picture = QVBoxLayout()
        self.hbox_picture.addWidget(self.pic)
        self.hbox_picture.setAlignment(QtCore.Qt.AlignHCenter)
        # main layout
        vbox_main = QVBoxLayout()
        vbox_main.addLayout(self.hbox_picture)
        vbox_main.addLayout(hbox_buttons)

        self.setLayout(vbox_main)

        # gradient effect background
        p = QtGui.QPalette()
        gradient = QtGui.QLinearGradient(0, 0, 0, 1000)
        gradient.setColorAt(0.0, QtGui.QColor(75, 75, 203))  # color up
        gradient.setColorAt(1.0, QtGui.QColor(240, 240, 240))  # color down
        p.setBrush(QtGui.QPalette.Window, QtGui.QBrush(gradient))
        self.setPalette(p)

        # Main Window
        self.setFixedSize(1200, 900)
        self.setWindowTitle('DRONE SURVEILLANCE')
        self.show()

    def connect_button_action(self):
        if not self.connected:
            try:
                self.cclient.start()
                #self.cclient.sendGlobalPolygon(gs.GLOBAL_AREA_POLYGON_PATH)
                self.connected = True
                self.set_connect_button_properties(connected=True)
                self.test_button.show()  # TEST
            except Exception as e:
                raise e
        else:
            self.connected = False
            self.cclient.stop()
            self.stop = True
            self.set_connect_button_properties(connected=False)

    def set_connect_button_properties(self, connected):
        if not connected:
            led_path = gs.RED_DOT_PATH
            button_text = "Connect"
            self.set_welcome_window()

        else:
            led_path = gs.GREEN_DOT_PATH
            button_text = "Disconnect"
            self.select_map_button.setEnabled(True)

        image = QtGui.QPixmap(led_path)
        self.connect_led.setPixmap((image.scaled(self.connect_led.width(), self.connect_led.height())))
        self.connect_button.setText(button_text)

    def select_map_button_action(self):
        # function that show select area map window
        self.select_area_window.show()

    def close_select_map_window(self):
        # function that close select area map window
        self.select_area_window.close()
        self.select_area_button.setEnabled(True)

    def select_area_button_action(self):
        self.hide()
        # Create object map_gui
        # shows fullscreen map image
        self.map_gui.set_picture(self.chosen_map_path)
        if self.map_gui.start_ui():
            self.map_gui.destroy_window()
            self.update_picture_frame(gs.GLOBAL_AREA_IMG_PATH_BLACKMASK)
            self.select_area_button.setDisabled(True)
            self.send_area_button.show()
        else:
            self.map_gui.destroy_window()
        self.show()

    def send_area_button_action(self):
        # Area partition and Drones path calculations
        # self.map_gui.flight_plans_calculating(gs.ALPHA, self.N)
        # Show Confirm window
        confirmation = QMessageBox.question(self, 'Confirm', "Are you sure?", QMessageBox.Yes | QMessageBox.No)
        if confirmation == QMessageBox.Yes:
            print("sending picture....")
            self.send_area_button.hide()
            self.select_area_button.hide()
            self.select_map_button.hide()
            self.cclient.sendGlobalPolygon(gs.GLOBAL_AREA_POLYGON_PATH)
            print("picture sent!")
            self.stop_button.show()
            self.show_hide_button.show()
            # self.start_thread_simulation()  # offline simulation
            self.start_test()
        else:
            # comeback to select area menu
            self.update_picture_frame(self.chosen_map_path)
            self.send_area_button.hide()
            self.select_area_button.setEnabled(True)

    def stop_button_action(self):
        self.stop = True
        self.set_menu_window()

    def show_hide_button_action(self):
        if self.show_flight_plan:
            self.show_flight_plan = False
        else:
            self.show_flight_plan = True
        self.map_gui.set_display_flight_plans(self.show_flight_plan)
        # self.area_reconstruction_position() # TEST
        self.area_reconstruction(self.current_area)  # OFFLINE SIMULATION
        self.update_picture_frame(gs.GLOBAL_AREA_IMG_PATH_RECONSTRUCTION)
        QApplication.processEvents()

    def start_test(self):
        self.stop=False
        print("starting test")
        last_all_nodes = dict()
        self.map_gui.reset_transparent_img()
        sleep(1)
        while (not self.stop):
            print("enter while")
            recv_allnodes = self.cclient.fetchAllNodes()
            print("pass fetch")
            if recv_allnodes != last_all_nodes:
                if len(last_all_nodes) != len(recv_allnodes):
                    # recalculate flight plans
                    print("recalculating flight plans")
                    self.N = len(recv_allnodes)
                    self.new_nodes = deepcopy(recv_allnodes)
                    # calculating flight plans in a new thread
                    #self.map_gui.flight_plans_calculating(gs.ALPHA, self.new_nodes)
                    #self.start_thread_calcul_flights()
                    threading.Thread(target=self.calcul_flight_plans).start()
                self.map_gui.area_reconstruction_position(recv_allnodes)
                if not self.stop:
                    self.update_picture_frame(gs.GLOBAL_AREA_IMG_PATH_RECONSTRUCTION)
                last_all_nodes = deepcopy(recv_allnodes)
            else:
                print("discarding")
            sleep(self.UPDATE_SLEEP_RATIO * gs.AUTOPILOT_SPEED)

    def start_thread_calcul_flights(self):
        print("starting calcul flights thread...")
        thread = CalculFlights(self)
        thread.start()

    def calcul_flight_plans(self):
        self.map_gui.set_display_flight_plans(False)
        self.map_gui.flight_plans_calculating(gs.ALPHA, self.new_nodes)
        self.map_gui.set_display_flight_plans(True)
        print("new flight plan ready")

    def start_test_button_action(self):
        self.map_gui.set_picture(gs.GLOBAL_AREA_IMAGE_TEST)
        self.map_gui.get_polygon(file_path=gs.GLOBAL_AREA_POLYGON_PATH)
        self.map_gui.start_ui_test()
        self.cclient.sendGlobalPolygon(gs.GLOBAL_AREA_POLYGON_PATH)
        self.start_test()

    def update_picture_frame(self, picture_filename):
        # update label pic in GUI
        threadLock.acquire()
        image = QtGui.QPixmap(picture_filename)
        self.pic.setPixmap(image.scaled(self.pic.width(), self.pic.height(), QtCore.Qt.KeepAspectRatio))
        self.pic.setAlignment(QtCore.Qt.AlignHCenter)
        self.hbox_picture.setAlignment(QtCore.Qt.AlignHCenter)
        QApplication.processEvents()
        threadLock.release()

    def set_welcome_window(self):
        self.select_map_button.show()
        self.select_area_button.show()
        self.select_map_button.setDisabled(True)
        self.select_area_button.setDisabled(True)
        self.send_area_button.hide()
        self.stop_button.hide()
        self.show_hide_button.hide()
        self.update_picture_frame(picture_filename=gs.MENU_WELCOME_PICTURE_PATH)

    def set_menu_window(self):
        self.select_map_button.show()
        self.select_area_button.show()
        self.select_map_button.setEnabled(True)
        self.select_area_button.setDisabled(True)
        self.stop_button.hide()
        self.show_hide_button.hide()
        self.update_picture_frame(picture_filename=gs.MENU_WELCOME_PICTURE_PATH)

    def closeEvent(self, event):
        # Show Confirm window
        confirmation = QMessageBox.question(self, 'Confirm', "Do you want to Exit?", QMessageBox.Yes | QMessageBox.No)
        if confirmation == QMessageBox.Yes:
            self.cclient.stop()
            self.stop = True
            event.accept()
        else:
            event.ignore()


######### USEFUL CLASSES ######

class SelectMapWindow(QDialog):
    """ QDialog Class that allows users to select area map"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.initUi()
        self.parent = parent

    def initUi(self):

        # labels
        self.map = []
        for i in range(gs.DATASET):
            image = QtGui.QPixmap(gs.MINI_MAP_PATH[i])
            self.map.append(QLabel(self))
            self.map[i].setFixedSize(100, 700)
            self.map[i].setPixmap(image)
            self.map[i].installEventFilter(self)

        # Layouts
        hbox_select_area = QHBoxLayout()
        for j in range(gs.DATASET):
            hbox_select_area.addWidget(self.map[j])
        self.setLayout(hbox_select_area)

        # window
        self.setWindowTitle("CHOOSE THE MAP")

    def eventFilter(self, object, event):
        if event.type() == QtCore.QEvent.MouseButtonDblClick:
            for i in range(gs.DATASET):
                if object.pixmap() == self.map[i].pixmap():
                    picture_path = gs.MAP_PATH[i]
                    self.parent.update_picture_frame(picture_path)
                    self.parent.chosen_map_path = picture_path
                    self.parent.close_select_map_window()
            return True
        else:
            return False


class CalculFlights(threading.Thread):
    """
         Thread Class used for the real time area view
    """

    def __init__(self, window):
        threading.Thread.__init__(self)
        self.window = window

    def run(self):
        self.window.calcul_flight_plans()


####### MAIN ################

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = UserGUI()
    sys.exit(app.exec_())
