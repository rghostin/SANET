#ifndef IMAGER_DATABASE_DB_UTILS_HPP
#define IMAGER_DATABASE_DB_UTILS_HPP

#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "loguru.hpp"
#include "Position.hpp"
#include "Image.hpp"


// SQLITE INTERNAL UTILS ================================================================

sqlite3 *dbOpen(const char *filename) {
    LOG_SCOPE_FUNCTION(INFO);
    sqlite3 *db;
    // The database is opened for reading and writing, and is created if it does not already exist.
    if (sqlite3_open(filename, &db)) {
        LOG_F(ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        throw;
    } else {
        LOG_F(INFO, "Opened database successfully");
    }
    return db;
}


void dbExecute(sqlite3 *db, const char *pSQL, int (*callback)(void *, int, char **, char **), void *data) {
    LOG_SCOPE_FUNCTION(INFO);
    char *szErrMsg = nullptr;
    if ((sqlite3_exec(db, pSQL, callback, data, &szErrMsg)) != SQLITE_OK) {
        LOG_F(ERROR, "SQL error: %s\n", szErrMsg);
        sqlite3_free(szErrMsg);
    }
}

struct data_display {
    std::vector<std::string> res;
};

static int dbCallback_display(void *data, int argc, char **argv, char **szColName) {
    data_display *datax = static_cast<data_display *>(data);
    for (unsigned short i = 0; i < argc; ++i) {
        std::string datap = argv[i];
        datax->res.push_back(datap);
    }
    return 0;
}


// DB SETUP ================================================================

void dbInstanciateTables(sqlite3 *db) {
    LOG_SCOPE_FUNCTION(INFO);
    // Create Node Status Table
    std::string SQL = "CREATE TABLE IF NOT EXISTS node_status_map (nodeID INTEGER PRIMARY KEY, latitude INTEGER, longitude INTEGER, timestamp INTEGER);";
    const char *pSQL = SQL.c_str();
    data_display mydata1;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata1);

    // Create Image Table
    SQL = "CREATE TABLE IF NOT EXISTS image_map (latitude INTEGER, longitude INTEGER, nodeID INTEGER, timestamp INTEGER, content TEXT, PRIMARY KEY (latitude, longitude));";
    pSQL = SQL.c_str();
    data_display mydata2;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata2);
}


// DB ADDING UTILS ================================================================

void dbInsertNode(sqlite3 *db, uint8_t nodeId, const Position &pos, uint32_t timestamp) {
    //
    LOG_SCOPE_FUNCTION(INFO);
    const char *pSQL;
    // Checks if Node already exists;
    data_display verify_exists;
    std::string node_id_str = std::to_string(nodeId);
    std::string SQL_verification = "SELECT * FROM node_status_map WHERE nodeID='" + node_id_str + "'";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    dbExecute(db, pSQL, dbCallback_display, (void *) &verify_exists);
    if (verify_exists.res.size() > 0) {
        LOG_F(INFO, "Tried to add nodeId %s to dbs BUT nodeId already exists.", node_id_str.c_str());
        throw;
    }
    std::string latitude_str = std::to_string(pos.latitude);
    std::string logitude_str = std::to_string(pos.longitude);
    std::string timestamp_str = std::to_string(timestamp);
    std::string SQL_insertion_in_users =
            "INSERT INTO node_status_map (nodeID,latitude,longitude,timestamp) VALUES ('" + node_id_str + "', '" +
            latitude_str + "', '" + logitude_str + "', '" + timestamp_str + "');";
    pSQL = SQL_insertion_in_users.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display mydata;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata);
    LOG_F(INFO, "Added nodeId %s to table 'node_status_map'", node_id_str.c_str());
}

void
dbInsertImage(sqlite3 *db, uint8_t nodeId, uint32_t timestamp, const Position &pos, const std::vector<char> &content) {
    LOG_SCOPE_FUNCTION(INFO);
    const char *pSQL;
    // Checks if Node already exists;
    data_display verify_exists;
    std::string node_id_str = std::to_string(nodeId);
    std::string latitude_str = std::to_string(pos.latitude);
    std::string logitude_str = std::to_string(pos.longitude);
    std::string timestamp_str = std::to_string(timestamp);
    std::string image_str(content.begin(), content.end());
    std::string SQL_verification =
            "SELECT * FROM image_map WHERE (latitude,longitude)=('" + latitude_str + "', '" + logitude_str + "');";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    dbExecute(db, pSQL, dbCallback_display, (void *) &verify_exists);
    if (verify_exists.res.size() > 0) {
        LOG_F(INFO, "Tried to add image to dbs BUT image already exists.");
        throw;
    }
    // else insert
    std::string SQL_insertion_in_users =
            "INSERT INTO image_map (latitude,longitude,nodeID,timestamp,content) VALUES ('" + latitude_str + "', '" +
            logitude_str + "', '" + node_id_str + "', '" + timestamp_str + "', '" + image_str + "');";
    pSQL = SQL_insertion_in_users.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display mydata;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata);
    LOG_F(INFO, "Added image for nodeId %s to table 'image_map'", node_id_str.c_str());
}


// DB REMOVE UTILS ================================================================

void dbRemoveNode(sqlite3 *db, uint8_t nodeId) {
    LOG_SCOPE_FUNCTION(INFO);
    const char *pSQL;
    std::string node_id_str = std::to_string(nodeId);
    std::string SQL_verification = "DELETE FROM node_status_map WHERE nodeID='" + node_id_str + "'";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display delete_node;
    dbExecute(db, pSQL, dbCallback_display, (void *) &delete_node);
}


void dbRemoveImage(sqlite3 *db, const Position &pos) {
    LOG_SCOPE_FUNCTION(INFO);
    const char *pSQL;
    std::string latitude_str = std::to_string(pos.latitude);
    std::string logitude_str = std::to_string(pos.longitude);
    std::string SQL_verification =
            "DELETE FROM image_map WHERE (latitude,longitude)=('" + latitude_str + "', '" + logitude_str + "');";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display delete_image;
    dbExecute(db, pSQL, dbCallback_display, (void *) &delete_image);
}



// DB UPDATING UTILS ================================================================

void dbInsertOrUpdateNode(sqlite3 *db, uint8_t nodeId, const Position &new_pos, uint32_t new_timestamp) {
    // Update Position, timestamp for a given nodeId.
    LOG_SCOPE_FUNCTION(INFO);
    const char *pSQL;
    // Checks if Node already exists;
    data_display verify_exists;
    std::string node_id_str = std::to_string(nodeId);
    std::string SQL_verification = "SELECT * FROM node_status_map WHERE nodeID='" + node_id_str + "'";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    dbExecute(db, pSQL, dbCallback_display, (void *) &verify_exists);
    if (verify_exists.res.size() <= 0) {
        LOG_F(INFO, "Tried to update nodeId %s to dbs BUT nodeId does not exists.", node_id_str.c_str());
        dbInsertNode(db, nodeId, new_pos, new_timestamp);
        return;
    }
    std::string latitude_str = std::to_string(new_pos.latitude);
    std::string logitude_str = std::to_string(new_pos.longitude);
    std::string timestamp_str = std::to_string(new_timestamp);
    std::string SQL_insertion_in_users =
            "UPDATE node_status_map (nodeID,latitude,longitude,timestamp) VALUES ('" + node_id_str + "', '" +
            latitude_str + "', '" + logitude_str + "', '" + timestamp_str + "') WHERE nodeID='" + node_id_str + "'";
    pSQL = SQL_insertion_in_users.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display mydata;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata);
    LOG_F(INFO, "Updated nodeId %s into table 'node_status_map'", node_id_str.c_str());
}


void dbInsertOrUpdateImage(sqlite3 *db, uint8_t new_nodeId, uint32_t new_timestamp, const Position &pos,
                           const std::vector<char> &new_content) {
    // Update nodeId, timestamp, image for a given Position.
    LOG_SCOPE_FUNCTION(INFO);
    const char *pSQL;
    // Checks if Node already exists;
    data_display verify_exists;
    std::string node_id_str = std::to_string(new_nodeId);
    std::string latitude_str = std::to_string(pos.latitude);
    std::string logitude_str = std::to_string(pos.longitude);
    std::string timestamp_str = std::to_string(new_timestamp);
    std::string image_str(new_content.begin(), new_content.end());
    std::string SQL_verification =
            "SELECT * FROM image_map WHERE (latitude,longitude)=('" + latitude_str + "', '" + logitude_str + "');";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    dbExecute(db, pSQL, dbCallback_display, (void *) &verify_exists);
    if (verify_exists.res.size() <= 0) {
        LOG_F(INFO, "Tried to add image to dbs BUT image already exists.");
        dbInsertImage(db, new_nodeId, new_timestamp, pos, new_content);
        return;
    }
    // else insert
    std::string SQL_insertion_in_users =
            "UPDATE image_map (latitude,longitude,nodeID,timestamp,content) VALUES ('" + latitude_str + "', '" +
            logitude_str + "', '" + node_id_str + "', '" + timestamp_str + "', '" + image_str +
            "') WHERE (latitude,longitude)=('" + latitude_str + "', '" + logitude_str + "');";
    pSQL = SQL_insertion_in_users.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display mydata;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata);
    LOG_F(INFO, "Added image for nodeId %s to table 'image_map'", node_id_str.c_str());
}


// DB GETTER UTILS ================================================================

const std::pair<Position, uint32_t> dbGetNodeStatus(sqlite3 *db, uint8_t nodeId) {
    // Get Position and Timestamp given a nodeID
    LOG_SCOPE_FUNCTION(INFO);
    std::pair<Position, uint32_t> node_status;
    Position position;
    uint32_t timestamp;

    const char *pSQL;
    std::string node_id_str = std::to_string(nodeId);
    std::string SQL_verification = "SELECT * FROM node_status_map WHERE nodeID='" + node_id_str + "'";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display mydata;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata);
    if (mydata.res.size() == 0) {
        LOG_F(INFO, "Tried to retrieve information, but nothing found");
        throw;
    }
    position.latitude = std::stoi(mydata.res[1]);
    position.longitude = std::stoi(mydata.res[2]);
    timestamp = std::stoi(mydata.res[3]);
    LOG_F(INFO, "Position=(%s,%s) and Timestamp=%s",
          std::to_string(position.latitude).c_str(),
          std::to_string(position.longitude).c_str(),
          std::to_string(timestamp).c_str());
    node_status = std::make_pair(position, timestamp);
    return node_status;
}

const Image dbGetImage(sqlite3 *db, const Position &pos) {
    // Get Image given a couple of points
    LOG_SCOPE_FUNCTION(INFO);

    const char *pSQL;
    std::string latitude_str = std::to_string(pos.latitude);
    std::string logitude_str = std::to_string(pos.longitude);
    std::string SQL_verification =
            "SELECT * FROM image_map WHERE (latitude,longitude)=('" + latitude_str + "', '" + logitude_str + "');";
    pSQL = SQL_verification.c_str();
    LOG_F(INFO, "Asking for the following SQL query: '%s'", pSQL);
    data_display mydata;
    dbExecute(db, pSQL, dbCallback_display, (void *) &mydata);
    if (mydata.res.size() == 0) {
        LOG_F(INFO, "Tried to retrieve information, but nothing found");
        throw;
    }
    // Retrieve (nodeID,timestamp,content) from result
    uint8_t nodeId = std::stoi(mydata.res[2]);
    uint32_t timestamp = std::stoi(mydata.res[3]);
    std::vector<char> content(mydata.res[4].begin(), mydata.res[4].end());
    LOG_F(INFO, "NodeId=%s, Timestamp=%s, Content=%s",
          std::to_string(nodeId).c_str(),
          std::to_string(timestamp).c_str(),
          mydata.res[4].c_str());
    return Image(nodeId, timestamp, pos, content);
}


#endif //IMAGER_DATABASE_DB_UTILS_HPP