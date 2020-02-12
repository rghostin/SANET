import matplotlib.pyplot as plt
from functools import reduce
import operator
import math
from sympy import Polygon

# EXCEPTIONS =========================================================
class InvalidPolygonFile(Exception):
    """Raised when the input value is too small"""
    pass


# PLOT UTILS =========================================================

class Colors:
    GREEN = (0, 255, 0)
    RED = (0, 0, 255)
    WHITE = (255, 255, 255)
    ORANGE = (255,140,0)


def plotPoly(poly_vertices, color):
    for i in range(len(poly_vertices) - 1):
        point = poly_vertices[i]
        next_point = poly_vertices[i + 1]
        plt.plot((point[0], next_point[0]), (point[1], next_point[1]), color=color)
    plt.plot((next_point[0], poly_vertices[0][0]), (next_point[1], poly_vertices[0][1]),
             color=color)


def plotFlightPlan(vertices, route):
    plotPoly(vertices, "black")
    plotPoly(route, "green")


def plotAllFlightPlans(flight_plans):
    for fp in flight_plans:
        vertices = fp.polygon_vertices
        route = fp.route
        plotFlightPlan(vertices, route)
    plt.show()

# GEOMETRY FUNCTIONS ==============================================================

def orderCoords(coords):
    center = tuple(
        map(operator.truediv, reduce(lambda x, y: map(operator.add, x, y), coords),
            [len(coords)] * 2))
    sorted_list = (sorted(coords, key=lambda coord: (-135 - math.degrees(
        math.atan2(*tuple(map(operator.sub, coord, center))[::-1]))) % 360))
    start_index = sorted_list.index(coords[0])
    res = [sorted_list[(i+start_index)% len(sorted_list)] for i in range(len(sorted_list))]
    return res



def parsePolygonFile(file_path):
    try:
        if file_path.split('.')[-1] != "polygon":
            raise InvalidPolygonFile
        else:
            vertices = list()
            with open(file_path, "r") as global_polygon:
                for line in global_polygon:
                    vertex = line.strip().split(',')
                    try:
                        vertices.append(tuple(map(int, vertex)))
                    except ValueError:
                        raise InvalidPolygonFile
            return Polygon(*vertices)
    except IOError:
        print("The filepath is not valid")
    except InvalidPolygonFile:
        print("Given file is not a '.polygon' file")
        print("Or, the global polygon file has an invalid format")
