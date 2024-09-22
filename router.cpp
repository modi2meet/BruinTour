#include "router.h"
#include "geotools.h"
#include <queue>
#include <unordered_map>

Router::Router(const GeoDatabaseBase& geo_db) : map(geo_db) {}

Router::~Router() {}

std::vector<GeoPoint> Router::route(const GeoPoint& pt1, const GeoPoint& pt2) const {
    std::priority_queue<Node*, std::vector<Node*>, NodeComparator> openSet;
    std::unordered_map<std::string, Node*> key; // Using string representation of GeoPoint as key

    Node* startNode = new Node(pt1, 0, heuristic(pt1, pt2), nullptr);
    openSet.push(startNode);
    key[pt1.to_string()] = startNode;

    while (!openSet.empty()) {
        Node* current = openSet.top();
        openSet.pop();

        if (current->point.to_string() == pt2.to_string()) {
            std::vector<GeoPoint> path = reconstructPath(current);
            // Clean up dynamically allocated nodes
            for (auto& entry : key) {
                delete entry.second;
            }
            return path;
        }

        for (const GeoPoint& neighbor : map.get_connected_points(current->point)) {
            double tentative_g_score = current->g_score + distance_earth_miles(current->point, neighbor); // Using miles for consistency

            if (!key.count(neighbor.to_string()) || tentative_g_score < key[neighbor.to_string()]->g_score) {
                Node* neighborNode = key[neighbor.to_string()];
                if (!neighborNode) {
                    neighborNode = new Node(neighbor, tentative_g_score, tentative_g_score + heuristic(neighbor, pt2), current);
                    key[neighbor.to_string()] = neighborNode;
                    openSet.push(neighborNode);
                } else {
                    neighborNode->g_score = tentative_g_score;
                    neighborNode->f_score = tentative_g_score + heuristic(neighbor, pt2);
                    neighborNode->parent = current;
                    // Since priority_queue doesn't support decrease-key operation, we push the updated node again.
                    openSet.push(neighborNode);
                }
            }
        }
    }

     //Clean up dynamically allocated nodes in case of no path found
    for (auto& entry : key) {
        delete entry.second;
    }
    return {}; // No path found
}

Router::Node::Node(const GeoPoint& p, double g, double f, Node* par) : point(p), g_score(g), f_score(f), parent(par) {}

bool Router::Node::operator>(const Node& other) const {
    return f_score > other.f_score;
}

bool Router::NodeComparator::operator()(const Node* a, const Node* b) const {
    return *a > *b;
}

double Router::heuristic(const GeoPoint& a, const GeoPoint& b) const {
    return distance_earth_miles(a, b); // Using miles for consistency
}

std::vector<GeoPoint> Router::reconstructPath(Node* currentNode) const {
    std::vector<GeoPoint> path;
    while (currentNode) {
        path.push_back(currentNode->point);
        currentNode = currentNode->parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}
