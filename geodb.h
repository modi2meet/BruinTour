#ifndef geodb_h
#define geodb_h

#include "base_classes.h"
#include "hashmap.h"

class GeoDatabase: public GeoDatabaseBase
{
public:
    GeoDatabase();
    virtual ~GeoDatabase();
    virtual bool load(const std::string& map_data_file);
    virtual bool get_poi_location(const std::string& poi, GeoPoint& point) const;
    virtual std::vector<GeoPoint> get_connected_points(const GeoPoint& pt) const; // geopoints
    virtual std::string get_street_name(const GeoPoint& pt1, const GeoPoint& pt2) const;
    

private:
    HashMap<GeoPoint> poiMap;
    HashMap<std::vector<GeoPoint>> connectionsMap; // Maps GeoPoints to vectors of connected GeoPoints
    HashMap<std::string> streetNamesMap; // Maps pairs of GeoPoints to street names // string to string. 
    void updateConnectionsMap(const GeoPoint& keyPoint, const GeoPoint& connectedPoint);
     

    
};

#endif /* geodb_h */
