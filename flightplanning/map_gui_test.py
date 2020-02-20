
import numpy as np
import cv2 as cv2
from sympy import Polygon
from operator import itemgetter
from utils import Colors
from utils import calcul_scope
from flight_planner import FlightPlanner
from copy import deepcopy



class MapGUI(object):
    """
    User interface that allows users:
    1) Select the desired area (polygon vertices) from a map via the select_area method.
    2) Show the area's partition and different paths within each partition via the drawFlightPlans method.
    """

    def __init__(self, window_name, points_filename, crop_filename,
                 crop_withblack_filename, crop_withfps_filename, reconstruct_area_filename,
                 drones_photo_path, display=False):
        self.display = display
        self.window_name = window_name  # Window's Name
        self.points_filename = points_filename
        self.crop_filename = crop_filename
        self.crop_withblack_filename = crop_withblack_filename
        self.crop_with_fps_filename = crop_withfps_filename
        self.reconstruct_filename = reconstruct_area_filename
        self.drones_photo_path = drones_photo_path
        self.done = False  # Flag signalling user is done choosing area points
        self.cancel = False # Flag signalling user cancel area_selection
        self.current = (0, 0)  # current user choosing point
        self.points = []  # List of points defining original polygon
        self.croped_polygon = None  # List of points defining croped polygon
        self.original_picture = None
        self.canvas = None  # charging full area map
        self.crop_picture = None  # selected area
        self.crop_withblack = None  # selected area with black
        self.crop_with_fps = None  # selected area with flight plans(drones paths)
        self.transparent_img = None # transparent layer used in area reconstruction
        self.current_status = None  # save last nodes_status received
        self.fplanner = None
        self.scope = None


    def start_ui(self):
        # start windows that allows users to select desired area
        # returns True if user successfully selected area, False if user cancel area selection
        self.select_area()
        if not self.cancel:
            self.crop_polygon()
            self.create_polygon_file()
            self.points = list()
            self.create_picture(filename=self.crop_filename, picture=self.crop_picture, to_png=False)
            self.create_picture(filename=self.crop_withblack_filename, picture=self.crop_withblack, to_png=True)
            return True
        else:
            return False

    def set_picture(self, path):
        self.original_picture = cv2.imread(filename=path)

    def mouse_clic_action(self, event, x, y, buttons, user_param):
        # Mouse callback that gets called for every mouse event (i.e. moving, clicking, etc.)
        if self.done:  # Nothing more to do
            return
        if event == cv2.EVENT_MOUSEMOVE:
            # Mouse move - update current mouse position
            self.current = (x, y)
        elif event == cv2.EVENT_LBUTTONDOWN:
            # Left click  - adding a point at current position to the list of vertices
            if self.display:
                print("Adding point #%d with position(%d,%d)" % (len(self.points), x, y))
            self.points.append((x, y))
            cv2.circle(img=self.canvas, center=(x, y), radius=40, color=Colors.RED, thickness=-1)
            cv2.polylines(img=self.canvas, pts=np.array([self.points]), isClosed=False, color=Colors.ORANGE,
                          thickness=10)

    def select_area(self):
        # Create original picture copy
        self.canvas = self.original_picture.copy()
        # reset flag done and cancel
        self.done = False
        self.cancel = False
        # Create window and set a mouse callback to handle events
        cv2.namedWindow(winname=self.window_name, flags=cv2.WINDOW_NORMAL)
        cv2.resizeWindow(winname=self.window_name, width=1000, height=800)
        cv2.imshow(winname=self.window_name, mat=self.canvas)
        cv2.waitKey(delay=1)
        cv2.setMouseCallback(self.window_name, self.mouse_clic_action)
        while (not self.done):
            # Selecting Area Loop
            # Update the window
            cv2.imshow(winname=self.window_name, mat=self.canvas)
            # And wait 50ms before next iteration (this will pump window messages meanwhile)
            key_pressed =cv2.waitKey(50)
            if key_pressed == 32:  # Press Enter to finish the area selection
                if Polygon(*self.points).is_convex():
                    self.done = True
                else:
                    print("Polygon is not convex. Test with another one.")
                    self.points = list()
                    self.cancel = True
                    # TODO: reset polylines
            elif key_pressed == 27:  # Press Esc to cancel area selection
                self.done = True
                self.cancel = True
                self.points = list()
        # of a filled polygon
        if (len(self.points) > 2) and not self.cancel:
            cv2.polylines(img=self.canvas, pts=np.array([self.points]), isClosed=True, color=Colors.ORANGE,
                          thickness=10)
            # TODO else when <= 2 points
            # TODO verify its not convex
            # show polygon
            cv2.imshow(winname=self.window_name, mat=self.canvas)
            # Waiting for the user to press any key
            cv2.waitKey()


    def check_selected_polygon(self):
        valid_polygon = False
        if(len(self.points)> 2):
            if Polygon(*self.points).is_convex():
                valid_polygon = True
            elif Polygon(*(self.points[::-1])).is_convex():  # check anticlockwise
                valid_polygon = True
                self.points = self.points[::-1]
            else:
                valid_polygon = False
        else:
            valid_polygon = False
        return valid_polygon


    def crop_polygon(self):
        ## CROP the selected desired area
        # crop map
        self.croped_polygon = np.array(self.points)
        x, y, crop_w, crop_h = cv2.boundingRect(self.croped_polygon)
        self.crop_picture = self.original_picture[y:y + crop_h, x:x + crop_w].copy()
        # mask
        self.croped_polygon = self.croped_polygon - self.croped_polygon.min(axis=0)
        mask = np.zeros(self.crop_picture.shape[:2], np.uint8)
        cv2.drawContours(image=mask, contours=[self.croped_polygon], contourIdx=-1, color=Colors.WHITE, thickness=-1,
                         lineType=cv2.LINE_AA)
        # transform PNG
        copie = self.crop_picture.copy()
        self.crop_withblack = self.create_png_image(image=copie, transparency=0)

        # apply mask
        self.crop_withblack[:, :, 3] = mask

    def create_png_image(self, image, transparency):
        b, g, r = cv2.split(image)
        alpha = np.ones(b.shape, dtype=b.dtype) * transparency
        png_image = cv2.merge((b, g, r, alpha))
        return png_image

    def create_polygon_file(self):
        # create a text file with the polygon's coordinates
        with open(file=self.points_filename, mode="w") as file:
            for pair in self.croped_polygon:
                file.write(str(pair[0]) + ", " + str(pair[1]) + "\n")

    def create_picture(self, filename, picture, to_png=True):
        # create an image
        if to_png:
            cv2.imwrite(filename=filename, img=picture)
        else:
            cv2.imwrite(filename=filename, img=picture, params=[int(cv2.IMWRITE_JPEG_QUALITY), 30])

    def flight_plans_calculating(self, alpha, status_node_map):
        # draws flight plans in the selected area
        # calcul scope
        self.scope = calcul_scope(self.crop_picture, alpha)
        print("SCOPE:",self.scope)
        # create object FlightPlanner
        self.fplanner = FlightPlanner(global_area_path=self.points_filename, scope=self.scope)
        self.fplanner.recompute(status_node_map)
        # draw flight plans
        png_crop_picture = self.create_png_image(image=self.crop_picture, transparency=255)
        self.crop_with_fps = self.draw_flight_plans(png_crop_picture)
        # create picture
        self.create_picture(filename=self.crop_with_fps_filename, picture=self.crop_with_fps, to_png=True)

    def draw_flight_plans(self, picture):
        for flight_plan in self.fplanner.flight_plans:
            # draw partition area
            polygon = np.array(flight_plan.polygon_vertices, np.int32)
            picture = cv2.polylines(img=picture, pts=[polygon], isClosed=True, color=Colors.WHITE_PNG, thickness=5)
            # draw route
            route = np.array(flight_plan.route, np.int32)
            picture = cv2.polylines(img=picture, pts=[route], isClosed=True, color=Colors.ORANGE_PNG, thickness=3)
            cv2.imwrite("test.png", picture)
        return picture

    def set_display_flight_plans(self, flag):
        if flag:
            self.display = True
        else:
            self.display = False

    def destroy_window(self):
        cv2.destroyWindow(winname=self.window_name)

    ############ TEST FONCTIONS ##################

    #### OFFLINE SIMULATION ######
    def simulate_drones_surveillance(self):
        # Simulation of all drones flying and taking pictures
        images = {}
        drone_id = 0
        for flight_plan in self.fplanner.flight_plans:
            # route of each drone
            age = 0
            route = np.array(flight_plan.route, np.int32)
            for points in route:
                # drone taking picture at each point
                photo = self.take_photo(points, self.scope,
                                self.drones_photo_path +"/photo_" + str(points[0]) + "_" + str(points[1]) + ".jpg")
                images[(points[0], points[1])]=(drone_id, photo, age)
                age+=1
            drone_id +=1
        print("taking pictures: done")
        return images

    def take_photo(self, position, scope, path=None):
        # Drone taking picture at given position
        # create photo scope
        x = max((position[0] - scope), 0)
        y = max((position[1] - scope), 0)
        # take photo
        photo = self.crop_picture[y:y + (2 * scope), x:x + (2 * scope)].copy()
        # save photo
        # cv2.imwrite(path, photo) # we dont save the pictures (just for the test)
        return photo

    def area_reconstruction(self, images):
        # reconstruction of global area using drones photos
        # create transparent picture
        img_height, img_width = self.crop_picture.shape[:2]
        self.transparent_img = np.zeros((img_height, img_width, 4), dtype=np.uint8)
        x_pos = y_pos = photo_height = photo_width = 0
        # reconstruction of pictures
        for drone_id in images:
            for photo in images[drone_id]:
                x_pos = max((photo[0][0]- self.scope), 0)
                y_pos = max((photo[0][1]- self.scope), 0)
                photo_height, photo_width = photo[1].shape[:2]
                png_photo = self.create_png_image(image=photo[1], transparency=255)
                self.transparent_img[y_pos:y_pos + photo_height, x_pos:x_pos + photo_width] = png_photo
            # create transparent picture and add drone
            drone = cv2.imread(self.drones_photo_path, -1)
            drone = cv2.resize(drone, (photo_width, photo_height))
            try:
                self.transparent_img[y_pos:y_pos + photo_height, x_pos:x_pos + photo_width] = deepcopy(drone)
            except ValueError as e:
                print("3!! Error")
                pass

        if self.display:
            # draw flight plans
            self.transparent_img = self.draw_flight_plans(self.transparent_img)

        self.create_picture(self.reconstruct_filename, self.transparent_img)

    def order_received_images(self, images):
        order_images = {}
        # order_images[drone_id] = (points, photo, age)
        for points in images:
            if images[points][0]  in order_images:
                order_images[images[points][0]].append((points, images[points][1], images[points][2]))
            else:
                order_images[images[points][0]] = []
                order_images[images[points][0]].append((points, images[points][1], images[points][2]))
        # order drones photos in age order
        lengths = []
        for drone_id in order_images:
            order_images[drone_id].sort(key=itemgetter(2))
            lengths.append(len(order_images[drone_id]))
        return order_images, lengths

    ###### ONLINE TEST #########
    def area_reconstruction_position(self, drones_path):
        # reconstruction of global area using drones photos
        # create transparent picture
        img_height, img_width = self.crop_picture.shape[:2]
        self.transparent_img = np.zeros((img_height, img_width, 4), dtype=np.uint8)
        # save last nodes_status
        last_nodes_status = None
        # draw path
        for nodes_status in drones_path:
            for node in nodes_status:
                # take photo
                position = [int(nodes_status[node][0]), int(nodes_status[node][1])]
                photo = self.take_photo(position=position, scope=self.scope)
                photo_height, photo_width = photo.shape[:2]
                x_pos = int(max((position[0] - self.scope), 0))
                y_pos = int(max((position[1] - self.scope), 0))
                png_photo = self.create_png_image(image=photo, transparency=255)
                self.transparent_img[y_pos:y_pos + photo_height, x_pos:x_pos + photo_width] = png_photo
            last_nodes_status = nodes_status
        # draw drones
        for node in last_nodes_status:
            x_pos = int(max((last_nodes_status[node][0] - self.scope), 0))
            y_pos = int(max((last_nodes_status[node][1] - self.scope), 0))
            photo_height = photo_width = 2*self.scope
            # create transparent picture and add drone
            drone = cv2.imread(self.drones_photo_path, -1)
            drone = cv2.resize(drone, (photo_width, photo_height))
            # draw drone
            self.transparent_img[y_pos:y_pos + photo_height, x_pos:x_pos + photo_width] = drone

        # draw flight plans
        if self.display:
            self.transparent_img = self.draw_flight_plans(self.transparent_img)
        self.create_picture(self.reconstruct_filename, self.transparent_img)

    def get_polygon(self, file_path):
        self.points = []
        with open(file_path, "r") as global_polygon:
            for line in global_polygon:
                vertex = line.strip().split(',')
                print(vertex)
                self.points.append([int(vertex[0]), int(vertex[1])])

    def start_ui_test(self):
        self.crop_polygon()
        self.points = list()
        self.create_picture(filename=self.crop_filename, picture=self.crop_picture, to_png=False)
        self.create_picture(filename=self.crop_withblack_filename, picture=self.crop_withblack, to_png=True)






        # ============================================================================


if __name__ == "__main__":
    WINDOW_TITLE = "CITY MAP"
    ORIGINAL_AREA_IMAGE_PATH = "image_set/map2.jpg"
    GLOBAL_AREA_POLYGON_PATH = "../global_area.polygon"
    GLOBAL_AREA_IMG_PATH = "cropped_images/croped_image.jpg"
    GLOBAL_AREA_IMG_PATH_BLACKMASK = "cropped_images/croped_image_black.jpg"


