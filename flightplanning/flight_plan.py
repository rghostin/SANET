class FlightPlan:
    def __init__(self, sub_polygon, scope):
        self.sub_polygon = sub_polygon
        self.polygon_vertices = [tuple(e) for e in self.sub_polygon.vertices]
        self.__route_ = None # List of tuples of int
        self.__encoded_json_ = None # Struct serializing the obj to send on network
        self.__scope_ = scope
        self.nodeid = None
        self.start_waypoint = None
    
    def __str__():
        return "FP(id=%d, start=%s, scope=%f, subpoly=%s" % (self.nodeid, str(self.start_waypoint), self.__scope_, ','.join([str(e) for e in self.sub_polygon]))

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

