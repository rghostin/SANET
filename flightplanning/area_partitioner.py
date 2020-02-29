from sympy import Point2D, Line2D, Polygon, Triangle, intersection
from utils import orderCoords, parsePolygonFile #plotPoly
#import matplotlib.pyplot as plt

colors = ['black', 'green', 'blue', 'yellow', 'red']


def find_x_coords(tta, chosen_vert, second_vert, m, b):
    x_1, y_1 = chosen_vert
    x_2, y_2 = second_vert
    a = ((x_1 * m) - (x_2 * m) - y_1 + y_2)
    b = ((- x_1 * b) + (x_2 * b) - (x_2 * y_1) + (x_1 * y_2))
    return (-(tta * 2) + b) / a, ((tta * 2) + b) / a


def find_y_coord(m, x_coord, b):
    return m * x_coord + b


def find_magic_point(original_poly, tta, chosen_vert, second_vert, third_vert, a_, b_,
                     c_):
    if b_ == 0:
        x_c = -(c_ / a_)
        x_1, y_1 = chosen_vert
        x_2, y_2 = second_vert
        to_add = ((-x_2 * y_1) + (x_c * y_1) + (x_1 * y_2) - (x_c * y_2))
        y_c = (2 * tta + to_add) / (x_1 - x_2)
        if not (original_poly.contains(Point2D(x_c, y_c))):
            y_c = ((-2 * tta) + to_add) / (x_1 - x_2)
    else:
        m = -(a_ / b_)
        b = -c_ / b_
        x_c, x_2 = find_x_coords(tta, chosen_vert, second_vert, m, b)
        y_c = find_y_coord(m, x_c, b)
        if not (original_poly.contains(Point2D(x_c, y_c))):
            return x_2, find_y_coord(m, x_2, b)
    return x_c, y_c


def find_triangle(polyg_coords, chosen_vertex, inverse, i):
    index_chosen = polyg_coords.index(chosen_vertex)
    triangle_vertices = [chosen_vertex]
    multiplier = (-1 if inverse else 1)
    for delta in range(1, 3):
        triangle_vertices.append(
            polyg_coords[(index_chosen + (multiplier * (delta + i))) % len(polyg_coords)])
    return Triangle(*triangle_vertices)


def fin_sub_poly(inverse, polyg_coords, chosen_vertex, target_area):
    sub_polygon, other = None, None
    i = 0
    temp_target_area = target_area
    curr_triangle = find_triangle(polyg_coords, chosen_vertex, inverse, 0)
    while sub_polygon is None:
        curr_triangle_area = abs(curr_triangle.area)
        if curr_triangle_area > temp_target_area:
            opposite_side = [tuple(v) for v in curr_triangle.vertices if
                             v != Point2D(*chosen_vertex)]
            a_, b_, c_ = Line2D(*opposite_side).coefficients
            x_c, y_c = find_magic_point(curr_triangle, temp_target_area, chosen_vertex,
                                        opposite_side[0],
                                        opposite_side[-1], a_, b_,
                                        c_)
            key_point = Point2D(x_c, y_c)
            if temp_target_area < target_area:
                previous_vertices_ = set()
                for t in range(i):
                    previous_triangle = find_triangle(polyg_coords, chosen_vertex, inverse, t)
                    for vertex in previous_triangle.vertices:
                        previous_vertices_.add(tuple(vertex))
                sub_poly_coords = orderCoords(list(previous_vertices_)+[key_point])
                poly_except_triangle = [v for v in polyg_coords if
                                        v not in sub_poly_coords]
            else:
                sub_poly_coords = [p for p in curr_triangle.vertices][:-1] + [key_point]
                sub_poly_to_tuple = [tuple(e) for e in sub_poly_coords]
                poly_except_triangle = [v for v in polyg_coords if
                                        v not in sub_poly_to_tuple]
            if len(sub_poly_coords) == 3:
                sub_polygon = Triangle(*sub_poly_coords)
            elif len(sub_poly_coords) > 3:
                sub_polygon = Polygon(*sub_poly_coords)
            other = Polygon(*(poly_except_triangle + [chosen_vertex, key_point]))
            """
            plotPoly(polyg_coords, 'red')
            plotPoly(sub_poly_coords, 'black')
            plt.show()
            """
            new_curr_triangle = list(curr_triangle.vertices)
            new_curr_triangle[1] = key_point
            curr_triangle = Triangle(*new_curr_triangle)
            temp_target_area = target_area
        elif curr_triangle_area < temp_target_area:
            temp_target_area -= curr_triangle_area
            i += 1
            curr_triangle = find_triangle(polyg_coords, chosen_vertex, inverse, i)
        else:
            sub_polygon = curr_triangle
            i += 1
            curr_triangle = find_triangle(polyg_coords, chosen_vertex, inverse, i)
            intersec = intersection(Polygon(*polyg_coords), sub_polygon)
            other = [p for p in Polygon(*polyg_coords).vertices if p not in sub_polygon]
            for point in intersec[0].points:
                if point not in intersec[1]:
                    other.append(point)
            for point in intersec[1].points:
                if point not in intersec[0]:
                    other.append(point)
            other = Polygon(*other)

    return sub_polygon, other


def getFairPartitioning(poly, n_partitions, display=False):
    polyg_coords = [tuple(e) for e in poly.vertices]
    if n_partitions < 1:
        raise ValueError
    elif n_partitions == 1:
        return [poly]
    else:
        polyg = Polygon(*polyg_coords)
        target_area = abs(polyg.area) / n_partitions
        chosen_vertex = polyg_coords[0]
        final_polygons = []
        original_polyg_coords = polyg_coords
        polyg_coords = orderCoords(polyg_coords)
        original_n_parition = n_partitions
        for i in range(original_n_parition - 1):
            sub_polygon, other = fin_sub_poly(not (i % 2 == 0), polyg_coords,
                                              chosen_vertex, target_area)
            final_polygons.append(sub_polygon)

            if i == original_n_parition - 2:
                other = Polygon(*orderCoords([tuple(v) for v in other.vertices]))
                final_polygons.append(other)
            polyg_coords = [tuple(e) for e in other.vertices]
            polyg_coords = orderCoords(polyg_coords)
            #plotPoly(original_polyg_coords, 'black')
            #plotPoly([tuple(e) for e in sub_polygon.vertices], 'black')
            n_partitions -= 1
            polyg = Polygon(*polyg_coords)
            target_area = abs(polyg.area) / n_partitions
            chosen_vertex = tuple(other.vertices[-1])
        if display:
            pass #plt.show()
        return final_polygons


if __name__ == "__main__":
    # polyg_coords = [(7, 0), (14, 2), (13, 8), (1, 8), (0, 2)]
    # polyg_coords = [(0,0), (4,9), (6,8), (9,1)]
    # polyg_coords = [(0, 26), (14, 22), (16, 0), (5, 0)]
    # polyg_coords = [(0, 26), (14, 22), (16, 0)] # Triangle
    # square = [(1, 1), (6, 1), (6, 6), (1, 6)]
    # concave = [(2,1), (9,1), (10, 3), (6,5), (5,3), (4,5), (1,3)]

    poly = Polygon(*[(202, 84), (0, 1660), (1084, 1706), (1227, 0)])
    # poly = parsePolygonFile("/Users/mdc/PycharmProjects/sanet/conf/global_area.polygon")
    # getFairPartitioning(poly, 2, True)
    for i in range(3, 6):
        t = getFairPartitioning(poly, i, True)
        print(t)
        print([abs(p.area) for p in t])
