#ifndef ROUTER_H
#define ROUTER_H

#include "geodb.h"
#include "base_classes.h"
#include <vector>

class Router : public RouterBase {
public:
    Router(const GeoDatabaseBase& geo_db);
    virtual ~Router();

    virtual std::vector<GeoPoint> route(const GeoPoint& pt1, const GeoPoint& pt2) const;

private:
    const GeoDatabaseBase& map;

    struct Node {
        GeoPoint point;
        double g_score;
        double f_score;
        Node* parent;

        Node(const GeoPoint& p, double g, double f, Node* par);

        bool operator>(const Node& other) const;
    };

    class NodeComparator {
    public:
        bool operator()(const Node* a, const Node* b) const;
    };

    double heuristic(const GeoPoint& a, const GeoPoint& b) const;
    std::vector<GeoPoint> reconstructPath(Node* currentNode) const;
};

#endif
