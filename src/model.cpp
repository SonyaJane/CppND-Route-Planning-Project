#include "model.h"
#include "pugixml.hpp"
#include <iostream>
#include <string_view>
#include <cmath>
#include <algorithm>
#include <assert.h>

// Helper function for LoadData()
// Returns a Road Type corresponding to the input string
static Model::Road::Type String2RoadType(std::string_view type)
{
    if( type == "motorway" )        return Model::Road::Motorway;
    if( type == "trunk" )           return Model::Road::Trunk;
    if( type == "primary" )         return Model::Road::Primary;
    if( type == "secondary" )       return Model::Road::Secondary;    
    if( type == "tertiary" )        return Model::Road::Tertiary;
    if( type == "residential" )     return Model::Road::Residential;
    if( type == "living_street" )   return Model::Road::Residential;    
    if( type == "service" )         return Model::Road::Service;
    if( type == "unclassified" )    return Model::Road::Unclassified;
    if( type == "footway" )         return Model::Road::Footway;
    if( type == "bridleway" )       return Model::Road::Footway;
    if( type == "steps" )           return Model::Road::Footway;
    if( type == "path" )            return Model::Road::Footway;
    if( type == "pedestrian" )      return Model::Road::Footway;    
    return Model::Road::Invalid;    
}

// Helper function for LoadData()
// Returns a Landuse Type corresponding to the input string
static Model::Landuse::Type String2LanduseType(std::string_view type)
{
    if( type == "commercial" )      return Model::Landuse::Commercial;
    if( type == "construction" )    return Model::Landuse::Construction;
    if( type == "grass" )           return Model::Landuse::Grass;
    if( type == "forest" )          return Model::Landuse::Forest;
    if( type == "industrial" )      return Model::Landuse::Industrial;
    if( type == "railway" )         return Model::Landuse::Railway;
    if( type == "residential" )     return Model::Landuse::Residential;    
    return Model::Landuse::Invalid;
}

// define the Model class constructor, which allows you to initialise a Model object with a reference to an 
// xml file that has been imported as a vector (sequence) of bytes. The const keyword indicates that the xml 
// parameter is read-only.
Model::Model( const std::vector<std::byte> &xml )
{
    LoadData(xml);

    AdjustCoordinates();

    std::sort(m_Roads.begin(), m_Roads.end(), [](const auto &_1st, const auto &_2nd){
        return (int)_1st.type < (int)_2nd.type; 
    });
}

// Builds data structures (m_Ways, m_Roads, m_Railways, etc.) by parsing information 
// from the elements in the OSM XML file. It populates these structures based on the 
// attributes and child elements of each element.
void Model::LoadData(const std::vector<std::byte> &xml)
{
    cout << "Reading OSM XML file and building data structures...\n";

    using namespace pugi; // Library for reading xml files
    
    xml_document doc; // pugi type

    if( !doc.load_buffer(xml.data(), xml.size()) )
        throw std::logic_error("failed to parse the xml file");
    
    // extract map bounds in terms of lattitude and longitude
    if( auto bounds = doc.select_nodes("/osm/bounds"); !bounds.empty() ) {
        auto node = bounds.first().node();
        m_MinLat = atof(node.attribute("minlat").as_string());
        m_MaxLat = atof(node.attribute("maxlat").as_string());
        m_MinLon = atof(node.attribute("minlon").as_string());
        m_MaxLon = atof(node.attribute("maxlon").as_string());
        m_bounds = {m_MinLat, m_MaxLat, m_MinLon, m_MaxLon};
    }
    else 
        throw std::logic_error("map's bounds are not defined");

    /*
    *****************************
    * m_Nodes                   *
    * mapping node IDs -> index *
    *****************************
     */
    // Create an unordered map (dictionary) named node_id_to_num to store a  
    // mapping between node IDs and corresponding indices (numbers).
    std::unordered_map<std::string, int> node_id_to_num;

    // Extract node IDs and coordnates (in terms of longitude and lattitude):
    // for each node in the xml document:
    for( const auto &node: doc.select_nodes("/osm/node") ) {
        // extract the value of the node "id" attribute and uses it as a key in the 
        // node_id_to_num map. The corresponding value is set to the current size of 
        // the m_Nodes vector, effectively assigning a unique number to each node ID.
        // The (int) part of (int)m_Nodes.size() converts the result of m_Nodes.size() to an integer.
        node_id_to_num[node.node().attribute("id").as_string()] = (int)m_Nodes.size();
        // A new (empty) Node object is added to the m_Nodes vector using emplace_back().
        m_Nodes.emplace_back();
        // The latitude ("lat") and longitude ("lon") attributes of the current node are 
        // extracted and converted to floating-point values using atof. These values are 
        // then assigned to the y and x members of the last added node, respectively.       
        m_Nodes.back().y = atof(node.node().attribute("lat").as_string());
        m_Nodes.back().x = atof(node.node().attribute("lon").as_string());
    }
    /*
    cout << "The number of nodes in the map is: " << m_Nodes.size() << '\n';
    cout << "The co-ordinates of the first 5 nodes, prior to adjustment, are:" << '\n';
    for (int i = 0; i < 5; i++) {
        cout << m_Nodes[i].x << ", " << m_Nodes[i].y << "\n";
    }
    */
    // Print the contents of the unordered map
    //for (const auto& pair : node_id_to_num) {
    //    std::cout << "Key (OSM ID): " << pair.first << ", Value (our ID): " << pair.second << std::endl;
    //}

    /*
    *****************************
    * m_Ways                    *
    * mapping way IDs -> index  *
    *****************************
     */

    // Create an unordered map (dictionary) named way_id_to_num to store a  
    // mapping between way IDs and corresponding indices (numbers).
    std::unordered_map<std::string, int> way_id_to_num;    

    // for each way element in the osm xml file:
    for( const auto &way: doc.select_nodes("/osm/way") ) {
        // extract the xml node:
        auto node = way.node();
        // assign a unique number to the way, and add an entry in way_id_to_num,  
        // mapping the way ID to the assigned number.
        // create a way id, called way_num, for this way: 
        const auto way_num = (int)m_Ways.size();
        way_id_to_num[node.attribute("id").as_string()] = way_num;
        // Add a new entry to the m_Ways vector and sets a reference (new_way) to the last added element.
        m_Ways.emplace_back();
        auto &new_way = m_Ways.back();

        // process child elements of the way (nodes and tags)
        for( auto child: node.children() ) {
            auto name = std::string_view{child.name()}; 

            // Extract the IDs of the nodes in the Way
            // If a child element is named "nd," get the node ID and add the corresponding 
            // node number to the nodes vector of the current way.
            if( name == "nd" ) {
                auto ref = child.attribute("ref").as_string();
                if( auto it = node_id_to_num.find(ref); it != end(node_id_to_num) )
                    new_way.nodes.emplace_back(it->second);
            }

            // Extract information about the way from the "tag" elements:
            // To build vectors m_Roads, m_Railways, m_Buildings, m_Leisures. m_Waters, m_Landuses 
            else if( name == "tag" ) {
                auto category = std::string_view{child.attribute("k").as_string()};
                auto type = std::string_view{child.attribute("v").as_string()};

                /*
                *****************************
                * m_Roads                   * 
                *****************************
                */
                if( category == "highway" ) {
                    if( auto road_type = String2RoadType(type); road_type != Road::Invalid ) {
                        m_Roads.emplace_back();
                        m_Roads.back().way = way_num;
                        m_Roads.back().type = road_type;
                    }
                }

                /*
                *****************************
                * m_Railways                * 
                *****************************
                */
                if( category == "railway" ) {
                    m_Railways.emplace_back();
                    m_Railways.back().way = way_num;
                }

                /*
                *****************************
                * m_Buildings               * 
                *****************************
                */                
                else if( category == "building" ) {
                    m_Buildings.emplace_back();
                    m_Buildings.back().outer = {way_num};
                }

                /*
                *****************************
                * m_Leisures                * 
                *****************************
                */
                else if( category == "leisure" ||
                        (category == "natural" && (type == "wood"  || type == "tree_row" || type == "scrub" || type == "grassland")) ||
                        (category == "landcover" && type == "grass" ) ) {
                    m_Leisures.emplace_back();
                    m_Leisures.back().outer = {way_num};
                }

                /*
                *****************************
                * m_Waters                   * 
                *****************************
                */
                else if( category == "natural" && type == "water" ) {
                    m_Waters.emplace_back();
                    m_Waters.back().outer = {way_num};
                }

                /*
                *****************************
                * m_Landuses                   * 
                *****************************
                */
                else if( category == "landuse" ) {
                    if( auto landuse_type = String2LanduseType(type); landuse_type != Landuse::Invalid ) {
                        m_Landuses.emplace_back();
                        m_Landuses.back().outer = {way_num};
                        m_Landuses.back().type = landuse_type;
                    }                    
                }
            }
        }
    }
    // for all "relation" elements
    for( const auto &relation: doc.select_nodes("/osm/relation") ) {
        // retrieve the corresponding XML node
        auto node = relation.node();
        // noode_id does not appear to be used anywhere
        //auto noode_id = std::string_view{node.attribute("id").as_string()};
        std::vector<int> outer, inner;
        // Define a lambda function named commit that takes a reference to a Multipolygon 
        // object (mp) and moves the contents of outer and inner vectors into the outer and 
        // inner members of the Multipolygon. This lambda function is used to consolidate
        // information about outer and inner rings.
        auto commit = [&](Multipolygon &mp) {
            mp.outer = std::move(outer);
            mp.inner = std::move(inner);
        };

        // go through all child elements of the relation element:
        for( auto child: node.children() ) {
            // get the name of the child element
            auto name = std::string_view{child.name()}; 
            // If the child element is "member":
            if( name == "member" ) {
                // get the "type" attribute (can be a node or a way, we're only interested in ways):
                // If the type is way:
                if( std::string_view{child.attribute("type").as_string()} == "way" ) {
                    // get the "ref" attribute and check if it is in the way_id_to_num dictionary
                    // counts number of times it occurs, if 0 then go to next child of the relation element
                    if( !way_id_to_num.count(child.attribute("ref").as_string()) )
                        continue;
                    // get the index cprresponding to the way ID
                    auto way_num = way_id_to_num[child.attribute("ref").as_string()];
                    // get the "role" attribute, to determine if the way ID is to be added to the outer or inner vector
                    if( std::string_view{child.attribute("role").as_string()} == "outer" )
                        outer.emplace_back(way_num);
                    else
                        inner.emplace_back(way_num);
                }
            }
            // tags determine where the inner and outer vectors will be pushed to
            // asumes tags are always after the members
            // If a child element is a tag, (note tags contain key value pairs, k and v)
            // processes different categories and types, adding information to 
            // vvectors (m_Buildings, m_Waters, m_Landuses, etc.) based on specific conditions. 
            // The commit lambda function is called to consolidate information.
            else if( name == "tag" ) { 
                // get key (category) value  (type) pair
                auto category = std::string_view{child.attribute("k").as_string()};
                auto type = std::string_view{child.attribute("v").as_string()};
                if( category == "building" ) {
                    commit( m_Buildings.emplace_back() );
                    break;
                }
                if( category == "natural" && type == "water" ) {
                    commit( m_Waters.emplace_back() );
                    BuildRings(m_Waters.back());
                    break;
                }
                if( category == "landuse" ) {
                    if( auto landuse_type = String2LanduseType(type); landuse_type != Landuse::Invalid ) {
                        commit( m_Landuses.emplace_back() );
                        m_Landuses.back().type = landuse_type;
                        BuildRings(m_Landuses.back());
                    }
                    break;
                }
            }
        }
    }
}

// convert node coordinates from Lattitude and Longitude to standardised coords, relative to the min lat and long (so min coords are 0,0)
void Model::AdjustCoordinates()
{    
    const auto pi = 3.14159265358979323846264338327950288;
    const auto deg_to_rad = 2. * pi / 360.;
    const auto earth_radius = 6378137.;
    const auto lat2ym = [&](double lat) { return log(tan(lat * deg_to_rad / 2 +  pi/4)) / 2 * earth_radius; };
    const auto lon2xm = [&](double lon) { return lon * deg_to_rad / 2 * earth_radius; };     
    const auto dx = lon2xm(m_MaxLon) - lon2xm(m_MinLon); // 749.18,
    const auto dy = lat2ym(m_MaxLat) - lat2ym(m_MinLat); // 578.759
    const auto min_x = lon2xm(m_MinLon); // -5440480
    const auto min_y = lat2ym(m_MinLat); // 1769190
    //cout << "min_x = " << min_x << ", min_y = " << min_y <<  '\n'; // 
    m_MetricScale = std::min(dx, dy); // 578.759
    //cout << "dx = "<< dx << ", dy = " << dy << '\n';
    for( auto &node: m_Nodes ) {
        node.x = (lon2xm(node.x) - min_x) / m_MetricScale;
        node.y = (lat2ym(node.y) - min_y) / m_MetricScale;        
    }
}

// Recursive helper function. Explores ways in the input vector open_ways to build a sequence of 
// connected nodes starting from an empty vector. It marks used ways in the used vector to avoid 
// revisiting them. The function tries to find a closed loop within the ways. If successful, it 
// returns true; otherwise, it returns false.
static bool TrackRec(const std::vector<int> &open_ways, // indices of open ways 
                     const Model::Way *ways,   // all ways in the model
                     std::vector<bool> &used,  // {false, ..., false} 
                     std::vector<int> &nodes)  // {}
{
    if( nodes.empty() ) { // If nodes is empty, it means the function is starting a new path.
        // loop over ways to find a starting way
        for( int i = 0; i < open_ways.size(); ++i ) // for each open way
            if( !used[i] ) {    // checks if the current way has not yet been used
                // mark the current way as used
                used[i] = true; 
                // get the nodes for the current way
                const auto &way_nodes = ways[open_ways[i]].nodes;
                // put them in the nodes vector
                nodes = way_nodes;
                // recursively call TrackRec with the updated parameters (updated by TrackRec)
                // If the recursive call eventually returns true, it means a closed loop has been found 
                if( TrackRec(open_ways, ways, used, nodes) )
                    return true;
                // If the recursive call did not find a closed loop, clear the nodes vector and mark the current way as unused.
                nodes.clear();
                used[i] = false;
            }
        // if no closed loop was found return false
        return false;
    }
    else {
        // if nodes is not empty we are continuing on an open path
        // get the first and last node of the current path
        const auto head = nodes.front(); 
        const auto tail = nodes.back(); 
        // if they are the same then path is closed so exit and return true
        if( head == tail && nodes.size() > 1 ) 
            return true;
        // if the path is not closed
        for( int i = 0; i < open_ways.size(); ++i ) // for each open way
            // if the current way hasn't been used
            if( !used[i] ) {  
                // Check if the current way connects to the last node of the current path: 
                // get the nodes for the current way                     
                const auto &way_nodes = ways[open_ways[i]].nodes; 
                // get the first node in the current way
                const auto way_head = way_nodes.front(); 
                // get the last node in the current way
                const auto way_tail = way_nodes.back();
                // if the first node in the current way is the same as the last node in the current path
                // or the last  node in the current way is the same as the last node in the current path:
                if( way_head == tail || way_tail == tail ) {
                    // mark the current way as used
                    used[i] = true;
                    // get the number of nodes in the current path                         
                    const auto len = nodes.size(); 
                    // if the first node in the current way is the same as the last node in the current path
                    if( way_head == tail )         
                        // add all the nodes in the current way to tbe end of the current path vector          
                        nodes.insert(nodes.end(), way_nodes.begin(), way_nodes.end());
                    // if the last  node in the current way is the same as the last node in the current path:
                    else
                        // reverse the order of the the nodes in the current way and add them to the end of the current path vector          
                        nodes.insert(nodes.end(), way_nodes.rbegin(), way_nodes.rend());
                    // Recursively calls the TrackRec function with the updated states
                    // If a closed loop is found return true
                    if( TrackRec(open_ways, ways, used, nodes) ) 
                        return true;
                    // if no closed loop was found
                    // return nodes to its original size, thus removing the nodes we just added from the current way
                    nodes.resize(len);   
                    // mark the current way to unused                 
                    used[i] = false;
                }
            }
        // If no closed loop was found starting from the current open way, return false.    
        return false;
    }
}

// input a vector of the indices of the open ways, and all the ways in the model
// Calls TrackRec to find a closed loop within the input open_ways. 
// If successful, it marks the used ways in open_ways and returns the closed loop nodes.
static std::vector<int> Track(std::vector<int> &open_ways, const Model::Way *ways)
{
    // make sure open_ways is not empty
    assert( !open_ways.empty() ); 
    // Create a boolean vector "used" of the same length as open_ways, with each entry = false.
    std::vector<bool> used(open_ways.size(), false); // {false, ...., false}
    // create empty vector to store the sequence of nodes forming a closed loop
    std::vector<int> nodes;                          // {}
    // call TrackRec to find a closed loop in the open_ways
    if( TrackRec(open_ways, ways, used, nodes) ) 
        // if a closed loop is found go through all the open ways       
        for( int i = 0; i < open_ways.size(); ++i )
            // if the open way was used in the path
            if( used[i] )
                // mark the open way as used by changing its index value to -1
                open_ways[i] = -1;
    // return the path that forms a closed loop
    return nodes;
}

// Used to ensure the inner and outer vectors of ways of a Multipolygon form a closed loop.
// This is used on Landuses and Waters found in the "relation" elements of the XML file 
// These are the only Multipolygons that can have more than one way in its inner and outer vectors.
void Model::BuildRings( Multipolygon &mp )
{
    // returns true if the nodes in the input way make a closed loop 
    auto is_closed = []( const Model::Way &way ) {
        // Returns true if the way has more than one node and the first node is the same as the last node
        return way.nodes.size() > 1 && way.nodes.front() == way.nodes.back();    
    };

    // For all input ways use function is_closed to determine if each way is closed or open
    auto process = [&]( std::vector<int> &ways_nums ) { 
        // get all of the ways in the model
        auto ways = m_Ways.data(); 
        // create empty vectors for sorting the ways into open and closed
        std::vector<int> closed, open; 
        
        // check if each way is open or closed, and adds it to either the open or closed vector
        for( auto &way_num: ways_nums ) // for all the ways in the mp vector 
            // determine if the way is open or closed
            (is_closed(ways[way_num]) ? closed : open).emplace_back(way_num);

        // while there are ways in the open vector:
        while( !open.empty() ) {    
            // call the Track function to find a closed loop in the ways
            auto new_nodes = Track(open, ways);
            if( new_nodes.empty() )
                break;
            // if a closed loop is found:
            // Recall the used open ways had their way IDs changed to 0 (in the open vector)
            // remove these -1 from the open vector
            open.erase(std::remove_if(open.begin(), open.end(), [](auto v){return v < 0;}), open.end() );
            // insert at the and of the closed vector the new way ID for the new way
            closed.emplace_back( (int)m_Ways.size() );
            // create a new way
            Model::Way new_way;
            // assign the found closed path to the new way
            new_way.nodes = std::move(new_nodes);
            // add the the way to the m_Ways vector
            m_Ways.emplace_back(new_way);
        }        
        // put all the IDs for the closed ways back into the input vector way_nums
        // This means we've removed any open ways  
        std::swap(ways_nums, closed);        
    };

    // find closed loops in the outer and inner ways
    process(mp.outer);
    process(mp.inner);
}