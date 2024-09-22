#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "geodb.h"
#include "geopoint.h"
#include "hashmap.h"
#include "geotools.h"
#include <utility> 

using namespace std;

GeoDatabase::GeoDatabase()
    : GeoDatabaseBase(),
      poiMap(0.75), // Initialize with the default load factor
      connectionsMap(0.75),
      streetNamesMap(0.75) {
}

// Destructor
GeoDatabase::~GeoDatabase() {
}

void GeoDatabase::updateConnectionsMap(const GeoPoint& keyPoint, const GeoPoint& connectedPoint) {
    auto existingPoints = connectionsMap.find(keyPoint.to_string());
    if (existingPoints != nullptr) {
        // Key exists, manually search for the connectedPoint in the existing vector
        bool found = false;
        for (const auto& point : *existingPoints) {
            if (point.to_string() == connectedPoint.to_string()) {
                found = true;
                break; // Connected point is already in the vector, no need to add it again
            }
        }
        if (!found) {
            existingPoints->push_back(connectedPoint); // Append the new connected point
        }
    } else {
        // Key doesn't exist, create a new entry with the connected point
        connectionsMap.insert(keyPoint.to_string(), std::vector<GeoPoint>{connectedPoint});
    }
}

vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}



bool GeoDatabase::load(const string& map_data_file) {
    ifstream file(map_data_file);
    if (!file.is_open()) {
        return false; // File not found
    }

    string line;
    while (getline(file, line)) {
        string streetName = line;
        
        getline(file, line);
        istringstream iss(line);
        string startLat, startLon, endLat, endLon;
        iss >> startLat >> startLon >> endLat >> endLon;
        GeoPoint startPoint = GeoPoint(startLat, startLon);
        GeoPoint endPoint = GeoPoint(endLat, endLon);
        vector<GeoPoint> startTemp;
        vector<GeoPoint> endTemp;
        vector<GeoPoint> midTemp;
        startTemp.push_back(endPoint);
        endTemp.push_back(startPoint);
        
        
        int numPOIs;
        file >> numPOIs;
        file.ignore();
        
        GeoPoint midPoint;
        if(numPOIs>0){
            // Calculate the midpoint for segments with POIs
            midPoint = midpoint(startPoint, endPoint);
            // Update connections to include the midpoint
            
            updateConnectionsMap(startPoint, midPoint); // Connect start to midpoint
            updateConnectionsMap(midPoint, startPoint); // Connect midpoint to start
            updateConnectionsMap(endPoint, midPoint);   // Connect end to midpoint
            updateConnectionsMap(midPoint, endPoint);   // Connect midpoint to end
        }
    
        updateConnectionsMap(startPoint, endPoint);
        updateConnectionsMap(endPoint, startPoint);
        
        if (numPOIs>0){
            string startMap1 = midPoint.to_string() + "-"+startPoint.to_string();
            string startMap2 = startPoint.to_string() + "-"+midPoint.to_string();
            string endMap1 = midPoint.to_string() + "-"+endPoint.to_string();
            string endMap2 =  endPoint.to_string() +"-"+ midPoint.to_string();
            streetNamesMap.insert(startMap1, streetName); // Add coordinates for streetname
            streetNamesMap.insert(startMap2, streetName); // Add coordinates for streetname
            streetNamesMap.insert(endMap1, streetName); // Add coordinates for streetname
            streetNamesMap.insert(endMap2, streetName); // Add coordinates for streetname

        }else{
            string Map1 = endPoint.to_string() + "-"+startPoint.to_string();
            string Map2 = startPoint.to_string() + "-"+endPoint.to_string();
            streetNamesMap.insert(Map1, streetName); // Add coordinates for streetname
            streetNamesMap.insert(Map2, streetName); // Add coordinates for streetname
        }


        
        for (int i = 0; i < numPOIs; ++i) {
            getline(file, line);
            istringstream poiIss(line);
            string poiName;
            string poiLat, poiLon;
            vector<string> parts = splitString(line, '|');
            
            poiName = parts[0];
            
            vector<string> points = splitString(parts[1], ' ');
            poiLat =points[0];
            poiLon = points[1];
            
            GeoPoint poiPoint = GeoPoint(poiLat, poiLon);
            poiMap.insert(poiName, poiPoint); // Add POI to the POI map

            // Connect POI to the midpoint of the segment it's on
            if (numPOIs > 0) {
                vector<GeoPoint> poiTemp;
                updateConnectionsMap(poiPoint, midPoint); // Connect POI to midpoint
                updateConnectionsMap(midPoint, poiPoint); // Connect midpoint to POI
                string poiMap1 = midPoint.to_string() + "-"+poiPoint.to_string();
                string poiMap2 = poiPoint.to_string() + "-"+midPoint.to_string();
                streetNamesMap.insert(poiMap1, "a path"); // Add coordinates for streetname
                streetNamesMap.insert(poiMap2, "a path"); // Add coordinates for streetname

            }
            
        }

    }

    file.close();
    return true;
}

bool GeoDatabase::get_poi_location(const std::string& poi, GeoPoint& point) const {
    const GeoPoint* poiPoint = poiMap.find(poi);
    if (poiPoint) {
        point = *poiPoint;
        return true;
    }
    return false;
}

std::vector<GeoPoint> GeoDatabase::get_connected_points(const GeoPoint& pt) const {
    const std::vector<GeoPoint>* connectedPoints = connectionsMap.find(pt.to_string());
    
    if (connectedPoints) {
        return *connectedPoints;
    }
    return {};
}

std::string GeoDatabase::get_street_name(const GeoPoint& pt1, const GeoPoint& pt2) const {
    std::string key1 = pt1.to_string() + "-" + pt2.to_string();
    std::string key2 = pt2.to_string() + "-" + pt1.to_string();

    const std::string* streetName = streetNamesMap.find(key1);
    if (streetName) {
        return *streetName;
    }

    streetName = streetNamesMap.find(key2);
    if (streetName) {
        return *streetName;
    }

    // Check for midpoint-based keys
    GeoPoint midPoint = midpoint(pt1, pt2);
    std::string midKey1 = midPoint.to_string() + "-" + pt1.to_string();
    std::string midKey2 = midPoint.to_string() + "-" + pt2.to_string();

    streetName = streetNamesMap.find(midKey1);
    if (streetName) {
        return *streetName;
    }

    streetName = streetNamesMap.find(midKey2);
    if (streetName) {
        return *streetName;
    }

    return "";
}

