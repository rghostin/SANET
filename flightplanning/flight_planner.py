from area_partitioner import getFairPartitioning
import numpy as np
from itertools import permutations
import math
from time import time
from utils import parsePolygonFile, plotAllFlightPlans, print_red
import json
import sys
from flight_plan import FlightPlan


class FlightPlanner:
    """
    Usage: fplanner.recompute(statusnodemap)
           fplanner.flight_plans
    """
    def __init__(self, global_area_path, scope):
        self.__num_partitions_ = None
        self._global_area_path = global_area_path
        self.__global_area_polygon_ = parsePolygonFile(global_area_path)
        self.__global_area_vertices_ = [tuple(e) for e in
                                        self.__global_area_polygon_.vertices]
        self.__individual_flight_plans_ = list()
        self.__scope_ = scope


    def notifyNewPolygon(self):
        print("Reading new polygon file")
        self.__global_area_polygon_ = parsePolygonFile(self._global_area_path)
        self.__global_area_vertices_ = [tuple(e) for e in
                                        self.__global_area_polygon_.vertices]

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
        print(47.5)
        checkpoints = np.full(
            (len(self.__individual_flight_plans_), len(drone_positions)), None)
        print(48)
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
        print(49)

        return distance_matrix, checkpoints

    def __computeBestAssignments_(self, drones_map):
        drones = {int(k):drones_map[k] for k in drones_map.keys()}
        print("drones->>>>", drones)
        drone_positions = list(drones.values())
        rev = {tuple(v): k for k, v in drones.items()}
        print('reversed---->', rev)

        print(drone_positions)
        distance_matrix, checkpoints = self.__computeDistanceMatrix_(drone_positions)
        print("45", drone_positions)
        min_sum = (sys.maxsize, None)
        all_poss = list(permutations([i for i in range(len(distance_matrix[0]))]))
        print(46)
        for poss in all_poss:
            s = sum(distance_matrix[fp][poss[fp]] for fp in
                    range(len(self.__individual_flight_plans_)))
            if s < min_sum[0]:
                min_sum = (s, poss)
        print(47)
        for i in range(len(drone_positions)):
            drone_pos_index = min_sum[1][i]
            print("Drone pos index", drone_pos_index)
            self.__individual_flight_plans_[i].nodeid = int(rev[tuple(drone_positions[drone_pos_index])])
            self.__individual_flight_plans_[i].start_waypoint = checkpoints[i][drone_pos_index]
            print_red("# Assinging fp %d to nodeid=", i, self.__individual_flight_plans_[i].nodeid)

        

    def recompute(self, status_nodemap):
        n = len(status_nodemap)
        assert n > 0
        if n != self.__num_partitions_:
            self.__individual_flight_plans_ = list()
            self.__encoded_structs_ = list()
            self.__num_partitions_ = n
            self.__computeSubPolygons_()  # fills subpolys in flightplan, remaining can be deduced inside the flighplan class
            self.__computeBestAssignments_(status_nodemap)
