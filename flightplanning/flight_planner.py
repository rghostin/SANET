from area_partitioner import getFairPartitioning
from find_TSP_solution import getBestRoute
import numpy as np
from itertools import permutations
import math
from time import time
from utils import parsePolygonFile, plotAllFlightPlans
import json
import sys

class FlightPlan:
    def __init__(self, sub_polygon, scope):
        self.sub_polygon = sub_polygon
        self.polygon_vertices = [tuple(e) for e in self.sub_polygon.vertices]
        self.__route_ = None # List of tuples of int
        self.__encoded_json_ = None # Struct serializing the obj to send on network
        self.__scope_ = scope
        self.nodeid = None
        self.start_waypoint = None

    @property
    def route(self):
        if self.__route_ is None:
            self.__route_ = getBestRoute(self.polygon_vertices, self.__scope_)
        return self.__route_

    @property
    def encoded_json(self):
        if self.__encoded_json_ is None:
            assert self.polygon_vertices is not None and self.__route_ is not None

            subregion_vertices = []
            for vert in self.polygon_vertices:
                subregion_vertices.append([float(coord) for coord in vert])

            to_send = {"polygon_vertices": subregion_vertices,
                       "route": self.__route_,
                       "drone_id": self.nodeid,
                       "start_pos": list(self.start_waypoint)}

            self.__encoded_json_ = json.dumps(to_send)
        return self.__encoded_json_


class FlightPlanner:
    """
    Usage: fplanner.recompute(statusnodemap)
           fplanner.flight_plans
    """
    def __init__(self, global_area_path, scope):
        self.__num_partitions_ = None
        self.__global_area_polygon_ = parsePolygonFile(global_area_path)
        self.__global_area_vertices_ = [tuple(e) for e in
                                        self.__global_area_polygon_.vertices]
        self.__individual_flight_plans_ = list()
        self.__scope_ = scope

    @property
    def flight_plans(self):
        return self.__individual_flight_plans_

    def __computeSubPolygons_(self):
        """
        Create flight plans with loaded subpoly and add to list
        FlightPlan is able to autonomously deduct route and encodage from subpoly
        """
        assert self.__num_partitions_
        sub_polygons = getFairPartitioning(self.__global_area_polygon_,
                                           self.__num_partitions_)

        for sub_poly in sub_polygons:
            self.__individual_flight_plans_.append(
                FlightPlan(sub_poly, self.__scope_))

    @staticmethod
    def __getIndicesOfTwoSmallest_(arr):
        return np.argpartition(np.array(arr), 2)[:2]

    def __computeDistanceMatrix_(self, drone_positions):
        distance_matrix = np.full(
            (len(self.__individual_flight_plans_), len(drone_positions)), 10000)
        checkpoints = np.full(
            (len(self.__individual_flight_plans_), len(drone_positions)), None)
        for fp_index in range(len(self.__individual_flight_plans_)):
            route = self.__individual_flight_plans_[fp_index].route
            for dr_index in range(len(drone_positions)):
                drone_pos = drone_positions[dr_index]
                for check_point in route:
                    dist_to_curr_point = math.sqrt(
                        sum([(a - b) ** 2 for a, b in zip(drone_pos, check_point)]))
                    if dist_to_curr_point < distance_matrix[fp_index][dr_index]:
                        distance_matrix[fp_index][int(dr_index)] = dist_to_curr_point
                        checkpoints[fp_index][dr_index] = check_point
        return distance_matrix, checkpoints

    def __computeBestAssignments_(self, drones):
        for i , droneid in enumerate(drones.keys()):
            self.__individual_flight_plans_[i].nodeid = droneid
            self.__individual_flight_plans_[i].start_waypoint = self.__individual_flight_plans_[i].route[0]

        """drone_positions = list(drones.values())
        rev = {tuple(v): k for k, v in drones.items()}
        distance_matrix, checkpoints = self.__computeDistanceMatrix_(drone_positions)
        min_sum = (sys.maxsize, None)
        all_poss = list(permutations([i for i in range(len(distance_matrix[0]))]))
        for poss in all_poss:
            s = sum(distance_matrix[fp][poss[fp]] for fp in
                    range(len(self.__individual_flight_plans_)))
            if s < min_sum[0]:
                min_sum = (s, poss)
        for i in range(len(drone_positions)):
            drone_pos_index = min_sum[1][i]
            self.__individual_flight_plans_[i].nodeid = rev[tuple(drone_positions[drone_pos_index])]
            self.__individual_flight_plans_[i].start_waypoint = checkpoints[i][drone_pos_index]
        """

    def recompute(self, status_nodemap):
        n = len(status_nodemap)
        assert n > 0
        if n != self.__num_partitions_:
            self.__individual_flight_plans_ = list()
            self.__encoded_structs_ = list()
            self.__num_partitions_ = n
            self.__computeSubPolygons_()  # fills subpolys in flightplan, remaining can be deduced inside the flighplan class
            self.__computeBestAssignments_(status_nodemap)
