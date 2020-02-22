import matplotlib.pyplot as plt
from functools import reduce
import operator
import math
from sympy import Polygon
import global_settings

# PARSING FUNCTIONS =============================================================

def parseNodeId(nodeId_path):
    with open(nodeId_path, "r") as nodeId_file:
        node_id = int(nodeId_file.readline().strip())
        if not(0 <= node_id <= 255):
            raise
        return node_id
    



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
    WHITE_PNG = (255, 255, 255, 255)
    ORANGE_PNG = (0, 165, 255, 255)


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


def print_red(s):
    print("\033[93m%s\033[0m" % s)


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
                        vertices.append(tuple(int(float(v.strip())) for v in vertex))
                    except ValueError:
                        raise InvalidPolygonFile
            print(vertices)
            return Polygon(*vertices)
    except IOError:
        print("The filepath is not valid")
    except InvalidPolygonFile:
        print("Given file is not a '.polygon' file")
        print("Or, the global polygon file has an invalid format")


def euclidian_distance(p1, p2):
    return math.sqrt( math.pow((p1[0] - p2[0]), 2) + math.pow((p1[1] - p2[1]), 2))
    


def calcul_scope(image, alpha):
    pixel_distance = max(image.shape[:2])
    scope = math.ceil(pixel_distance / (2 * alpha))
    return scope

def create_dic_bidon(N):
    dic = {}
    for i in range(N):
        dic[i]=(i+2, i+4)
    return dic

 
void json_write_poly_to_file(const std::string& json_poly, const char* fname) {
    FILE* polygon_file(fopen(fname, "wb")); 

    char buffer[2048];
    memset(buffer, '\0', sizeof(buffer));

    std::stringstream ss(json_poly);
    std::string temp;

    for (int i = 0; i < std::count(json_poly.begin(), json_poly.end(), '['); ++i) {
        std::getline(ss, temp, '[');
        std::getline(ss, temp, ']');
        temp += "\n";
        fwrite(temp.c_str(), sizeof(char), temp.size(), polygon_file);
    }
    fclose(polygon_file);
}
