#ifndef TOUR_GENERATOR_H
#define TOUR_GENERATOR_H

#include "base_classes.h"
#include "stops.h"
#include <vector>

class TourGenerator : public TourGeneratorBase {
public:
    TourGenerator(const GeoDatabaseBase& geodb, const RouterBase& router);
    virtual ~TourGenerator();

    virtual std::vector<TourCommand> generate_tour(const Stops& stops) const;

private:
    const GeoDatabaseBase& geodb_;
    const RouterBase& router_;

    void generateRouteCommands(const std::vector<GeoPoint>& route, std::vector<TourCommand>& result) const;
    std::string CompassDirection(double angle) const;
};

#endif // TOUR_GENERATOR_H
