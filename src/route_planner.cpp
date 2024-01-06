#include "route_planner.h"
#include <algorithm>

// : m_Model(model): is the member initializer list. It initializes the member variable m_Model with the provided model. 
RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to proportion:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    // Find the closest nodes to the start and end coordinates.
    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);

    cout << "start_node: index = " << start_node->Index() <<  "co-ordinates = (" << start_node->x << ", " << start_node->y << ")\n";
    cout << "end_node: index = " << end_node->Index() <<  "co-ordinates = (" << end_node->x << ", " << end_node->y << ")\n";

    // set g and h values and mark as visited
    start_node->g_value = 0.0;
    start_node->h_value = CalculateHValue(start_node);
    start_node->visited = true;
}

float RoutePlanner::CalculateHValue(RouteModel::Node* const node) {
    // distance to end Node
    return node->distance(*end_node);
}


// For the current node add all its unvisited neighbors to the open list
void RoutePlanner::AddNeighbors(RouteModel::Node* current_node) {
    //cout << "current_node index: " << current_node->Index() << '\n';
    // populate current_node.neighbors vector with all its neighbors:
    current_node->FindNeighbors();
    // go through all the neighbours:
    for (RouteModel::Node* node : current_node->neighbors){
        if (node->visited == false){
            //cout << "Neighbour node index: " << node->Index() << " ";
            // set the parent:
            node->parent = current_node;
            // set the h-value:
            node->h_value = CalculateHValue(node);
            // set the g-value
            node->g_value = current_node->g_value + current_node->distance(*node); // dereference node as its a pointer
            // set visited to true
            node->visited = true;
            // at it to the open_list
            open_list.emplace_back(node); // open_list is a vector or pointers to RouteModel::Node objects
            // cout << "Neighbour: node index: " << node->Index() << "parent node: " << node->parent->Index() << '\n';
        }
    }
    //cout << "open_list: \n";
    //for (RouteModel::Node* noode : open_list) {
    //    cout << noode->Index() << " ";
    //}
    //cout << '\n';
}

// Get a pointer to the next_node
RouteModel::Node* RoutePlanner::NextNode() {
    auto compare = [&](const RouteModel::Node* node1, const RouteModel::Node* node2){
        return ((node1->g_value + node1->h_value) > (node2->g_value + node2->h_value));
    };
    // Sort the open_list according to f = h + g, in descending order
    std::sort(this->open_list.begin(), this->open_list.end(), compare);

    //cout << "open_list: ";
    //for (const RouteModel::Node* noode : this->open_list){
    //    cout << noode->Index() << ", f = " << (noode->g_value + noode->h_value) << ", g = " << noode->g_value << ", h = " << noode->h_value << '\n';
    //}
    //cout << '\n';
    // Create a pointer to the node in the list with the lowest sum
    RouteModel::Node* next_node = (this->open_list).back(); 
    // Remove that node from the open_list
    (this->open_list).pop_back();
    //cout << "next_node: " << next_node->Index() << '\n';
    return next_node;
}


// Returns the final path found from the A* search.
// Input: the current (final) node a
//iteratively follow the 
//   chain of parents of nodes until the starting node is found.
// - For each node in the chain, add the distance from the node to its parent to the distance variable.
// - The returned vector should be in the correct order: the start node should be the first element
//   of the vector, the end node should be the last element.

std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node* current_node) {
    // Create path_found vector
    distance = 0.0f;
    std::vector<RouteModel::Node> path_found;

    while (current_node->Index() != start_node->Index() ){
        path_found.emplace(path_found.begin(), *current_node);
        // add distance from current_node to its parent
        distance += current_node->distance(*(current_node->parent));
        // set the current_node equal to the parent
        current_node = current_node->parent;
    }
    // add start node
    path_found.emplace(path_found.begin(), *start_node);

    distance *= m_Model.MetricScale(); // Multiply the distance by the scale of the map to get meters.
    cout << "distance: " << distance << '\n';
    return path_found;
}


// TODO 7: Write the A* Search algorithm here.
// Tips:
// - Use the AddNeighbors method to add all of the neighbors of the current node to the open_list.
// - Use the NextNode() method to sort the open_list and return the next node.
// - When the search has reached the end_node, use the ConstructFinalPath method to return the final path that was found.
// - Store the final path in the m_Model.path attribute before the method exits. This path will then be displayed on the map tile.

void RoutePlanner::AStarSearch() {
    RouteModel::Node* current_node = nullptr;

    current_node = start_node;

    cout << "current node: " << current_node->Index() << ", ";

    // do until current_node = end_node
    while (current_node->Index() != end_node->Index()){
        // add all of the neighbors of the current node to the open_list
        AddNeighbors(current_node);

        // if there are nodes in the open_list    
        if (open_list.size() > 0 ){    
            // do we need to update the g-values?

            // sort the open_list and return the next node
            current_node = NextNode();
            cout << current_node->Index() << ", ";
        }
        else {
            // we aren't at the end node and there are no more nodes to explore
            cout << "No path found!";
            break;
        }
    }
    m_Model.path = ConstructFinalPath(current_node);
    cout << "Path found: ";    
    for (RouteModel::Node node : m_Model.path){
        cout << node.Index() << " ";
    }
    cout << '\n';
}
