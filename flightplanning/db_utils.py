import sqlite3
from time import time
 
def db_update_currPos(con, nodeID, longitude, latitude):
    curr_timestamp = int(time())
    query = "INSERT OR REPLACE INTO node_status_map (nodeID,latitude,longitude,timestamp) VALUES (?, ?, ?, ?);"
    cursorObj = con.cursor()
    cursorObj.execute(query, (nodeID, latitude, longitude, curr_timestamp))
    con.commit()
