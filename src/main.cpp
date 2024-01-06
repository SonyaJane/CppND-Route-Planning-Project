#include <optional>  // std::nullopt
#include <fstream>   // file streaming classes
#include <iostream>
#include <vector>
#include <string>
#include <io2d.h>   // for displaying the route on a map
#include "route_model.h"
#include "render.h"
#include "route_planner.h"
#include "utility_route_model.h"

using namespace std::experimental;
/*
// ReadFile() outputs a vector of raw memory
static std::optional<std::vector<std::byte>> ReadFile(const std::string &path)
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
*/
int main(int argc, const char **argv)
{   
    // ***********************************************************************************************************
    // * READ IN DATA                                                                                            *
    // ***********************************************************************************************************

    // name of osm data file, which is in json format 
    std::string osm_data_file = "";

    // if this program is run at the command line with the name of an osm data file:   
    if( argc > 1 ) {
        for( int i = 1; i < argc; ++i )
            // parse the command line arguments
            if( std::string_view{argv[i]} == "-f" && ++i < argc )
                osm_data_file = argv[i];
    }
    else {
        std::cout << "To specify a map file use the following format: " << std::endl;
        std::cout << "Usage: [executable] [-f filename.osm]" << std::endl; // -f allows you to specify the osm data file 
        osm_data_file = "../map.osm"; // if you dont specify an osm data file it will be set to map.osm
    }
    
    std::vector<std::byte> osm_data; // create an empty vector of bytes
    
    // read the contents of the osm data file into the vector osm_data
    if( osm_data.empty() && !osm_data_file.empty() ) {
        std::cout << "Reading OpenStreetMap data from the following file: " <<  osm_data_file << std::endl;
        auto data = ReadFile(osm_data_file);
        if( !data )
            std::cout << "Failed to read." << std::endl;
        else
            osm_data = std::move(*data);
    }
    
    // ***********************************************************************************************************
    // * GET START AND END COORDINATES                                                                           *
    // ***********************************************************************************************************

    // Declare floats `start_x`, `start_y`, `end_x`, and `end_y` and get user input for these values 
    float start_x; float start_y; float end_x; float end_y;

    std::cout << "Please enter the start coordinates: " << std::endl;
    std::cin >> start_x >> start_y;
    std::cout << "Please enter the destination coordinates: " << std::endl;
    std::cin >> end_x >> end_y;

    // ***********************************************************************************************************
    // * CREATE MODEL                                                                                            *
    // ***********************************************************************************************************

    // Build model: create a RouteModel object. called model.          This data structure holds all of the OSM data in a convenient 
    // format, and provides some methods for using the data.
    RouteModel model{osm_data};

    // ***********************************************************************************************************
    // * CREATE ROUTE PLANNER                                                                                            *
    // ***********************************************************************************************************

    // create a RoutePlaner object using the model created above with user input start and end coordinates
    RoutePlanner route_planner{model, start_x, start_y, end_x, end_y};

    // perform A* search and save the results in the RoutePlaner object
    route_planner.AStarSearch();

    std::cout << "Distance: " << route_planner.GetDistance() << " meters. \n";

    // Render results of search - creates a render object using the model
    Render render{model};

    // display the results using the io2d library
    auto display = io2d::output_surface{400, 400, io2d::format::argb32, io2d::scaling::none, io2d::refresh_style::fixed, 30};
    display.size_change_callback([](io2d::output_surface& surface){
        surface.dimensions(surface.display_dimensions());
    });
    display.draw_callback([&](io2d::output_surface& surface){
        render.Display(surface);
    });
    display.begin_show();
}
