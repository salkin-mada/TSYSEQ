#include <Arduino.h>
#include <Entropy.h>
#include <Vector>


template <class T>
class MarkovNode {
public:
typedef MarkovNode<T> Node;
typedef Node* NodePtr;
private:
  struct Connection {
    NodePtr node;
    float probability;
    Connection() : node(NULL), probability(0) {}
    Connection(NodePtr n, float prob) : node(n), probability(prob) {} 
  };
  float probability_accumulator;
  unsigned int connectioncount;
  Connection connections[8];
public:
  T data;
  void addConnection(NodePtr node, float prob)
  {
    //Connection c(node, prob);
    //connections.push_back(c);
    connections[connectioncount].node = node;
    connections[connectioncount].probability = prob;
    probability_accumulator += prob;
    ++connectioncount;
  }
  NodePtr activate(){
    //
    float r = Entropy.randomf() * probability_accumulator;
    float pa = 0;
    int match = -1;
    for (unsigned int i = 0; i < connectioncount; ++i){
      //if (pa >= r);
      pa += connections[i].probability;
      if (pa >= r) {
        match = i;
        break;
      }
    }
    
    return connections[match].node;
  }
  MarkovNode()
  {
    //
    connectioncount = 0;
    probability_accumulator = 0;
    
  }
};

template <class T>
class Markov {
private:
  MarkovNode<T> *current;
  MarkovNode<T> network[32];
public:
  void setData(unsigned int index, T data)
  {
    network[index].data = data;
  }
  void connect(unsigned int index1, unsigned int index2, float prob)
  {
    network[index1].addConnection(&network[index2], prob);
  }
  MarkovNode<T> *next()
  {
    if (current == NULL) {
      current = &network[0];
    } else {
      current = current->activate();
    }
    return current;
  }
  Markov()
  {
    Entropy.Initialize();
    current = NULL;
  }
};

