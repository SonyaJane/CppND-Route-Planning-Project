#ifndef ROUTE_MODEL_H
#define ROUTE_MODEL_H

#include <limits>
#include <cmath>
#include <unordered_map>
#include "model.h"
#include <iostream>

// A RouteModel object is created with OSM data.
// It is a child of the Model class, from which it inherits everything (methods and attributes)
// it cannot directly access the private members of the parent class, Model
// It inherits publicly which means that the public and protected members of the base class (Model) become public members of RouteModel
class RouteModel : public Model {

  public:
    // Node class, which is child of the Model::Node struct.
    // This means it inherits members from Model::Node.
    // it is a nested class, which means it can access both public and private member of the RouteModel class
    class Node : public Model::Node {
      public:
        Node* parent = nullptr;
        float h_value = std::numeric_limits<float>::max(); // returns the maximum finite representable value for the float data type
        float g_value = 0.0;
        bool visited = false;
        std::vector<Node*> neighbors; // declares a vector that will store pointers to neighbouring nodes (Node objects)

        void FindNeighbors();

        float distance(Node other) const {
            return std::sqrt(std::pow((x - other.x), 2) + std::pow((y - other.y), 2));
        }
      
        auto &Index() const noexcept { return index; }

	      // class constructors - allows a Node object to be initialised with data (variables)
	
	      // for initialising an empty node
        Node(){}
        
        // Node constructor = uses an initialiser list
        Node(int idx, RouteModel* search_model, Model::Node node) : Model::Node(node), parent_model(search_model), index(idx) {}
        // Model::Node(node) copies the Node "node" (copy constructor) to obtain its attributes (.x and .y)
        // Copy constructors are the member functions of a class that initialize the data members of the class using another object of the same class. It copies the values of the data variables of one object of a class to the data members of another object of the same class.
        // parent_model(search_model) initializes the parent_model variable of the Node class with the value of the search_model parameter.
        // index(idx)                 initialises the index variable      

      private:
        int index;
        Node* FindNeighbor(std::vector<int> node_indices);
        // pointer to a RouteModel object
        RouteModel* parent_model = nullptr;
    };

    // RouteModel constructor (defined in cpp file)
    RouteModel(const std::vector<std::byte> &xml);
    Node &FindClosestNode(float x, float y);
    auto &SNodes() { return m_Nodes; }
    std::vector<Node> path;
    
  private:
    void CreateNodeToRoadHashmap();
    std::unordered_map<int, std::vector<const Model::Road*>> node_to_road;
    std::vector<Node> m_Nodes;

};

#endif
