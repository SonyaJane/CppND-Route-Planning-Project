#include "route_model.h"
#include <iostream>
#include <map>

// Define the class methods. When the class methods are defined outside the class, the 
// scope resolution operator :: must be used to indicate which class the method belongs to.
// This prevents any compiler issues if there are two classes with methods that have the same name. 

// constructor - allows you to instantiate a new RouteModel object
// It takes a reference to a vector of bytes (xml) and initializes the RouteModel object by 
// calling the constructor of its base class, Model, with the same xml parameter. The base class 
// Model is constructed first, and then the constructor body for RouteModel continues.
RouteModel::RouteModel(const std::vector<std::byte> &xml) : Model(xml) {
    // Create RouteModel nodes.
    int counter = 0; // will be used to assign unique indices to the nodes being created.
    // Iterate over the vector of nodes (m_Nodes) in the base class, Nodes, which were returned by calling the Model::Nodes() member function 
    for (Model::Node node : Nodes()) {
        // create new type of Nodes with additional attributes (in addition to .x and .y) and member functions:
        // Recall constructor: Node(int idx, RouteModel* search_model, Model::Node node
        // counter = new node index (same as old node index),  this = this instance of a RouteModel object, node = original node  
        m_Nodes.emplace_back(Node(counter, this, node));
        counter++;
    }
    CreateNodeToRoadHashmap();

    /*
    // print created dictionary
    std::cout << "NodeToRoadHashmap:\n";

    // sort the keys before printing:
    std::map<int, std::vector<const Model::Road*>> sorted_node_to_road(node_to_road.begin(), node_to_road.end());

    for (const auto& pair : sorted_node_to_road) {
        std::cout << pair.first << ": ";
        for (auto& road: pair.second){  // roads are memory addresses
            cout << (*road).way << " "; // prints road way ID
        }
        cout << '\n';
    }

    cout << "Number of nodes in dictionary: " << node_to_road.size() << '\n';
    */
}


void RouteModel::CreateNodeToRoadHashmap() {
    // iterate over the vector of roads (m_Roads) in the base class:
    for (const Model::Road &road : Roads()) {
        // roads have a way ID, "way" and a type, Type (one of 10 options)
        // If the road is not a footway:
        if (road.type != Model::Road::Type::Footway) {
            // get the nodes that correspond to the current road and iterate over them  
            for (int node_idx : Ways()[road.way].nodes) {
                // recall, at creation, node_to_road is a dictionary key: int, value: vector<const Model::Road*> (vector of pointers to Road objects)
                // .find() either points to sought-after element, or end() if not found.
                // If the current node index is not already a key in the dict 
                if (node_to_road.find(node_idx) == node_to_road.end()) {
                    // adds a new entry with an empty vector as the initial value
                    node_to_road[node_idx] = std::vector<const Model::Road*> ();
                }
                // append the current road pointer to the vector associated with the node index
                node_to_road[node_idx].push_back(&road);
            }
        }
    }
}

// finds neighbours of a node (my_node) given the IDs of nodes that are on the same roads as my_node
RouteModel::Node* RouteModel::Node::FindNeighbor(std::vector<int> node_indices) {
    // Initialise a pointer to a Node object and set it to nullptr. 
    // This pointer will eventually point to the closest node in the loop.
    Node *closest_node = nullptr;
    // create a local Node object called node
    // It is used to store the node obtained from the parent_model during each iteration
    Node node;

    // interate through each node in node_indices
    for (int node_index : node_indices) {
        // parent_model is a pointer to the parent RouteModel object
        // parent_model->SNodes() accesses the SNodes() member function of the parent_model
        // which returns a reference to the nodes vector in the parent model. 
        // get the current node
        node = parent_model->SNodes()[node_index];
        // this->distance(node) accesses the distance() member function of the RouteModel object
        // make sure the current node is not my_node and the node has not been visited
        if (this->distance(node) != 0 && !node.visited) {
            // if we haven't found a closest node yet, or the distance between my_node and the current node 
            // is less than that for the current closest_node, then set closest_node = current node
            if (closest_node == nullptr || this->distance(node) < this->distance(*closest_node)) {
                //
                closest_node = &parent_model->SNodes()[node_index];
            }
        }
    }
    return closest_node;
}

// finds neighbours of a node, let's say "my_node" and puts them in the neighbours attribute 
void RouteModel::Node::FindNeighbors() {                         // my_node.FindNeighbours()
    // iterate over all the roads that contain my_node. parent_model->node_to_road means use the 
    // node_to_road dictionary that belongs to the RouteModel object, parent_model
    for (auto &road : parent_model->node_to_road[this->index]) { // 'this' is my_node, my_node->index = my_node.index
        // call FindNeighbor() on my_node with input = nodes in the same roads as my_node. to get these node IDs:
        // for the current road get the way ID, and then find the node IDs that belong to the way
        RouteModel::Node* new_neighbor = this->FindNeighbor(parent_model->Ways()[road->way].nodes);
        if (new_neighbor) {
            this->neighbors.emplace_back(new_neighbor);
        }
    }
}

// m_Model.FindClosestNode mwill be used to find the closest nodes to the starting and ending coordinates.
RouteModel::Node& RouteModel::FindClosestNode(float x, float y) {
    Node input;
    input.x = x;
    input.y = y;

    float min_dist = std::numeric_limits<float>::max();
    float dist;
    int closest_idx;

    // for each road that isn't a Footway
    for (const Model::Road &road : Roads()) {
        if (road.type != Model::Road::Type::Footway) {
            // for each node in the road
            for (int node_idx : Ways()[road.way].nodes) {
                // calculate the distance between the input coordinates and the current node
                dist = input.distance(SNodes()[node_idx]);
                // if its less than the current minimum distance
                if (dist < min_dist) {
                    closest_idx = node_idx;
                    min_dist = dist;
                }
            }
        }
    }
    //SNodes() was defined in the header file as:
    // auto& SNodes() { return m_Nodes; }
    // returns reference to the vector of nodes, m_Nodes
    return SNodes()[closest_idx]; // returns reference to element of m_Nodes vector
}
