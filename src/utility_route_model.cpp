#include "utility_route_model.h"
#include <iostream>
#include <fstream>   // file streaming classes

using namespace std;

// ReadFile() outputs a vector of raw memory
std::optional<std::vector<std::byte>> ReadFile(const std::string& path)
{   
    // create input file stream: opens file for reading. binary - open in binary mode (read data in 
    // binary format (bytes)); ate - at the end, immediately go to the end of the input stream (so 
    // we can use tellg to determine the size of the input stream)
    std::ifstream is{path, std::ios::binary | std::ios::ate};

    // if no stream is created, std::nullopt conveys the absence of a value
    if( !is )
        return std::nullopt;
    
    // determine the size of the input stream (returns the input position indicator)
    auto size = is.tellg();

    // create a vector to contain the contents of the file in byte format
    std::vector<std::byte> contents(size);    
    
    // go to the start of the stream (seekg() sets the input position indicator)
    is.seekg(0);

    // read everthing into the contents vector. read() parameters:
    // s - pointer to the character array to store the characters to
    // count - number of characters to read    
    // vector.data() returns a pointer to the underlying element storage. For non-empty containers, 
    // the returned pointer is the address of the first element.  
    is.read((char*)contents.data(), size);

    // if the contents vector is empty return nothingness
    if( contents.empty() )
        return std::nullopt;

    // return the contents vector, move allows you to trasnfer the contents of the vector to another variable without using pointers or references    
    return std::move(contents);
}

const char* RoadTypeToString(Model::Road::Type t) noexcept
{
    switch (t)
    {
        case Model::Road::Motorway: return "Motorway";
        case Model::Road::Trunk: return "Trunk";
        case Model::Road::Primary: return "Primary";
        case Model::Road::Secondary: return "Secondary";
        case Model::Road::Tertiary: return "Tertiary";
        case Model::Road::Residential: return "Residential";
        case Model::Road::Service: return "Service";
        case Model::Road::Unclassified: return "Unclassified";
        case Model::Road::Footway: return "Footway";
        default: return "Unknown";
    }
}

const char* LanduseTypeToString(Model::Landuse::Type t) noexcept
{
    switch (t)
    {
        case Model::Landuse::Invalid: return "Invalid:";
        case Model::Landuse::Commercial: return "Commercial";
        case Model::Landuse::Construction: return "Construction";
        case Model::Landuse::Grass: return "Grass";
        case Model::Landuse::Forest: return "Forest";
        case Model::Landuse::Residential: return "Residential";
        case Model::Landuse::Industrial: return "Industrial";
        case Model::Landuse::Railway: return "Railway";
        default: return "Unknown";
    }
}

void print_nodes(RouteModel &model, int n)
{
    int n_nodes = model.Nodes().size();
    int m = min(n, n_nodes);
    cout << "\nThe number of Nodes in the map is: " << n_nodes << '\n';
    cout << "The co-ordinates of the first " << m << " nodes, after adjustment, are:" << '\n';
    for (int i = 0; i < m; i++) {
        cout << "(" << model.Nodes()[i].x << ", " << model.Nodes()[i].y << ")" << '\n';
    }
}

void print_roads(RouteModel &model, int n)
{
    int n_Roads = model.Roads().size();
    int m = min(n, n_Roads);
    cout << "\nThe number of Roads in the map is: " << n_Roads << '\n';
    cout << "The first " << m << " Roads are:" << '\n';
    cout << "(Way ID, Road type)" << '\n';

    for (int i = 0; i < m; i++) {
        cout << "{" << model.Roads()[i].way << ", " << RoadTypeToString(model.Roads()[i].type) << "}\n";
    }
}

void print_ways(RouteModel &model, int n)
{
    int n_ways = model.Ways().size();
    int m = min(n, n_ways);
    cout << "\nThe number of Ways in the map is: " << n_ways << '\n';
    cout << "The node IDs of the first " << m << " Ways are:" << '\n';

    for (int i = 0; i < m; i++) {
        int p = model.Ways()[i].nodes.size();
        cout << "{";
        for (int j = 0; j < p; j++){
            cout << model.Ways()[i].nodes[j]; 
            if (j < p-1) {
                cout << ", ";
            }
        }
        cout << "}\n";
    }
}

void print_railways(RouteModel &model, int n)
{
    int n_railways = model.Railways().size();
    int m = min(n, n_railways);
    // Railways have a single way ID
    cout << "\nThe number of Railways in the map is: " << n_railways << endl;
    if (n_railways > 0) {
        cout << "The Way IDs of the first " << n << " Ways are:" << '\n';
        for (int i; i < m; i++){
            cout << model.Railways()[i].way << '\n';
        }
    }
}

void print_buildings(RouteModel &model, int n)
{
    int n_buildings = model.Buildings().size();
    int m = min(n, n_buildings);
    // Buildings are a type of Multipolygon
    // They have a vector of outer way IDs (typically just 1) and a vector of inner way IDs (typically 0)
    cout << "The number of Buildings in the map is: " << n_buildings << '\n';
    cout << "The first " << m << " Buildings have the following way IDs:" << '\n';
    for (int i = 0; i < m; i++) {
        cout << "inner: ";
        int q = model.Buildings()[i].inner.size();
        for (int j = 0; j < q; j++){
            cout << model.Buildings()[i].inner[j] << ' ';
        }
        cout << "outer: " ;       
        int p = model.Buildings()[i].outer.size();
        for (int j = 0; j < p; j++){
            cout << model.Buildings()[i].outer[j] << ' ';
        }
        cout << '\n';
    }
    
}

void print_leisures(RouteModel &model, int n)
{
    int n_leisures = model.Leisures().size();
    int m = min(n, n_leisures);
    // Leisures are a type of Multipolygon
    // They have a vector of outer way IDs (typically just 1) and a vector of inner way IDs (typically 0)
    cout << "The number of Leisures in the map is: " << n_leisures << '\n';
    cout << "The first " << m << " Leisures have the following way IDs:" << '\n';
    for (int i = 0; i < m; i++) {
        cout << "inner: ";
        int q = model.Leisures()[i].inner.size();
        for (int j = 0; j < q; j++){
            cout << model.Leisures()[i].inner[j] << ' ';
        }
        cout << "outer: " ;       
        int p = model.Leisures()[i].outer.size();
        for (int j = 0; j < p; j++){
            cout << model.Leisures()[i].outer[j] << ' ';
        }
        cout << '\n';
    }
}

void print_waters(RouteModel &model, int n)
{
    int n_waters = model.Waters().size();
    int m = min(n, n_waters);
    // Waters are a type of Multipolygon
    // They have a vector of outer way IDs (typically just 1) and a vector of inner way IDs (typically 0)
    cout << "The number of Waters in the map is: " << n_waters << '\n';
    cout << "The first " << m << " Waters have the following way IDs:" << '\n';
    for (int i = 0; i < m; i++) {
        cout << "inner: ";
        int q = model.Waters()[i].inner.size();
        for (int j = 0; j < q; j++){
            cout << model.Waters()[i].inner[j] << ' ';
        }
        cout << "outer: " ;       
        int p = model.Waters()[i].outer.size();
        for (int j = 0; j < p; j++){
            cout << model.Waters()[i].outer[j] << ' ';
        }
        cout << '\n';
    }
    
}

void print_landuses(RouteModel &model, int n)
{
    int n_landuses = model.Landuses().size();
    int m = min(n, n_landuses);
    // Landuses are a type of Multipolygon
    // They have a vector of outer way IDs (typically just 1) and a vector of inner way IDs (typically 0)
    // They also have a type
    cout << "The number of Landuses in the map is: " << n_landuses << '\n';
    cout << "The first " << m << " Landuses have the following way IDs and type:" << '\n';
    for (int i = 0; i < m; i++) {
        cout << "inner: ";
        int q = model.Landuses()[i].inner.size();
        for (int j = 0; j < q; j++){
            cout << model.Landuses()[i].inner[j] << ' ';
        }
        cout << "outer: " ;       
        int p = model.Landuses()[i].outer.size();
        for (int j = 0; j < p; j++){
            cout << model.Landuses()[i].outer[j] << ' ';
        }
        cout << "Type: " << LanduseTypeToString(model.Landuses()[i].type);
        cout << '\n';
    }
    
}