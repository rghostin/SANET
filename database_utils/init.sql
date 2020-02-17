CREATE TABLE IF NOT EXISTS node_status_map 
    (nodeID INTEGER PRIMARY KEY, latitude INTEGER, longitude INTEGER, timestamp INTEGER);

CREATE TABLE IF NOT EXISTS image_map 
    (latitude INTEGER, longitude INTEGER, nodeID INTEGER, timestamp INTEGER, content TEXT, PRIMARY KEY (latitude, longitude));
