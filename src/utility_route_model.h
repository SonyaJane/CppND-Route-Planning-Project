#ifndef utility_route_model_hpp
#define utility_route_model_hpp

#include "route_model.h"
#include <optional>  // std::nullopt
#include <string>
#include <vector>

std::optional<std::vector<std::byte>> ReadFile(const std::string& path);
const char* RoadTypeToString(Model::Road::Type) noexcept;
const char* LanduseTypeToString(Model::Landuse::Type t) noexcept;

void print_nodes(RouteModel &model, int n);
void print_roads(RouteModel &model, int n);
void print_ways(RouteModel &model, int n);
void print_railways(RouteModel &model, int n);
void print_buildings(RouteModel &model, int n);
void print_leisures(RouteModel &model, int n);
void print_waters(RouteModel &model, int n);
void print_landuses(RouteModel &model, int n);

#endif