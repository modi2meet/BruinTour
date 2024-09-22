#include "tour_generator.h"
#include "geotools.h"
#include <cmath>


TourGenerator::TourGenerator(const GeoDatabaseBase& geodb, const RouterBase& router)
    : geodb_(geodb), router_(router) {}

TourGenerator::~TourGenerator() {}

std::vector<TourCommand> TourGenerator::generate_tour(const Stops& stops) const {
    std::vector<TourCommand> result;

    for (int i = 0; i < stops.size(); ++i) {
        std::string poi_name, description;
        if (stops.get_poi_data(i, poi_name, description)) {
            // Generate commentary command
            TourCommand commentary_command;
            commentary_command.init_commentary(poi_name, description);
            result.push_back(commentary_command);

            if (i < stops.size() - 1) {
                std::string next_poi_name, next_description;
                if (stops.get_poi_data(i + 1, next_poi_name, next_description)) {
                    GeoPoint current_point, next_point;
                    if (geodb_.get_poi_location(poi_name, current_point) &&
                        geodb_.get_poi_location(next_poi_name, next_point)) {
                        auto route = router_.route(current_point, next_point);
                        // Check if a route couldn't be generated
                        if (route.empty()) {
                            // Return an empty vector if no route is found
                            return std::vector<TourCommand>();
                        }
                        
                        generateRouteCommands(route, result);
                    }
                }
            }
        }
    }

    return result;
}

void TourGenerator::generateRouteCommands(const std::vector<GeoPoint>& route, std::vector<TourCommand>& result) const {
    for (size_t i = 0; i < route.size() - 1; ++i) {
        const auto& p1 = route[i];
        const auto& p2 = route[i + 1];
        std::string street_name = geodb_.get_street_name(p1, p2);
        double distance = 0;
        distance = distance_earth_miles(p1, p2);
        double angle = angle_of_line(p1, p2);
        std::string direction = CompassDirection(angle);

        TourCommand proceed_command;
        proceed_command.init_proceed(direction, street_name, distance, p1, p2);
        result.push_back(proceed_command);

        if (i < route.size() - 2) {
            const auto& p3 = route[i + 2];
            double turn_angle = angle_of_turn(p1, p2, p3);
            if (turn_angle >= 1.0 && turn_angle <= 359.0) {
                std::string next_street_name = geodb_.get_street_name(p2, p3);
                if (next_street_name != street_name) {
                    std::string turn_direction = (turn_angle >= 180.0) ? "right" : "left"; // onto new street
                    TourCommand turn_command;
                    turn_command.init_turn(turn_direction, next_street_name);
                    result.push_back(turn_command);
                }
            }
        }
    }
}

std::string TourGenerator::CompassDirection(double angle) const {
    //as required by the specifications
    if (angle >= 0 && angle < 22.5)
        return "east";
    else if (angle >= 22.5 && angle < 67.5)
        return "northeast";
    else if (angle >= 67.5 && angle < 112.5)
        return "north";
    else if (angle >= 112.5 && angle < 157.5)
        return "northwest";
    else if (angle >= 157.5 && angle < 202.5)
        return "west";
    else if (angle >= 202.5 && angle < 247.5)
        return "southwest";
    else if (angle >= 247.5 && angle < 292.5)
        return "south";
    else if (angle >= 292.5 && angle < 337.5)
        return "southeast";
    else
        return "east";
}
