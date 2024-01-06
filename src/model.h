#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>

using namespace std;

class Model
{
public:
    // A struct is a user-defined data type that can contain multiple variables of different types.
    // a Node has an x and a y value
    struct Node {
        // define the 2 variables
        double x = 0.f;  // declare a double variable named x and initialise it with a value of 0.f, which is a floating point literal that represents the value 0.0
        double y = 0.f;  // using double instead of float allows for greater precision in representing decimal values.
        // these attributes can be accessed as node_name.x, node_name.y
    };
    
    // A Way has a sequence of nodes, and it has tags. The Way struct contains the former.
    // Way has a vector of integers named nodes
    // The tags tell you what kind of way it is (road, railway, building, leisure, water, landuse)
    struct Way {
        std::vector<int> nodes;
    };
    
    // a Road represents a road segment, which has a road type, and a way ID
    // road types. used in function String2RoadType() in mode.cpp
    struct Road {
        // declare an enumeration type called Type, with different types of roads       
        enum Type { Invalid, Unclassified, Service, Residential,
            Tertiary, Secondary, Primary, Trunk, Motorway, Footway };
        int way; // declare an int variable named way. Represents a way ID associated with the road segment
        Type type; // declare a variable named type of the Type enumeration type (as defined 3 lines above)
    };
    
    // a Railway has a way ID
    struct Railway {
        int way;
    };    
    
    // a multipolygon consists of two polygons, an outer polygon and an inner one, each represented by a 
    // vector of way IDs
    struct Multipolygon {
        std::vector<int> outer;
        std::vector<int> inner;
    };
    
    // a building is a type of Multipolygon (child of a Multipolygon)
    struct Building : Multipolygon {};
    
    struct Leisure : Multipolygon {};
    
    struct Water : Multipolygon {};
    
    // Land types used in function String2LanduseType() in model.cpp   
    struct Landuse : Multipolygon {
        // access using Model::Landuse::Type
        enum Type { Invalid, Commercial, Construction, Grass, Forest, Industrial, Railway, Residential };
        Type type;
    };
    
    // Model class constructor, which allows you to initialise a Model object with a reference to an 
    // xml file that has been imported as a vector (sequence) of bytes. The const keyword indicates that the xml 
    // parameter is read-only.
    Model( const std::vector<std::byte> &xml );
    
    // declare member functions

    auto MetricScale() const noexcept { return m_MetricScale; } 
    
    // define member functions. They take no arguments. 
    // auto means the compiler will automatically deduce the return type.
    // & declares a reference to the return value of the function
    // const means the functions do not modify the object on which it is called. 
    // noexcept indicates that the function does not throw any exceptions.
    // { return m_Nodes; }:is the body of the member function. It simply returns the reference to the m_Nodes member variable.
    auto &Nodes() const noexcept { return m_Nodes; }
    auto &Ways() const noexcept { return m_Ways; }
    auto &Roads() const noexcept { return m_Roads; }
    auto &Buildings() const noexcept { return m_Buildings; }
    auto &Leisures() const noexcept { return m_Leisures; }
    auto &Waters() const noexcept { return m_Waters; }
    auto &Landuses() const noexcept { return m_Landuses; }
    auto &Railways() const noexcept { return m_Railways; }

    auto &Bounds() const noexcept { return m_bounds; };
    
private:
    // private member functions
    void AdjustCoordinates();
    void BuildRings( Multipolygon &mp );
    void LoadData(const std::vector<std::byte> &xml);
    
    // class attributes

    std::vector<Node> m_Nodes;  // vector of Node data structures (each has an x and a y coordinate)
    std::vector<Way> m_Ways;    // 
    std::vector<Road> m_Roads;
    std::vector<Railway> m_Railways;
    std::vector<Building> m_Buildings;
    std::vector<Leisure> m_Leisures;
    std::vector<Water> m_Waters;
    std::vector<Landuse> m_Landuses;
    
    double m_MinLat = 0.;
    double m_MaxLat = 0.;
    double m_MinLon = 0.;
    double m_MaxLon = 0.;
    double m_MetricScale = 1.f;

    std::vector<double> m_bounds {};
};
