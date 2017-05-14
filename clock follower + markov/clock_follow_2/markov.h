#include "emicros.h"
#include <vector>

class MarkovNode {
public:
  struct MarkovNodeConnection {
    MarkovNode * node;
    float probability;
    MarkovNodeConnection(MarkovNode * n, float p) : node(n), probability(p) {}
  };
private:
  std::vector<MarkovNodeConnection> forwardConnections;
  float probability_accum;
  unsigned int data;
public:
  inline void add(MarkovNode * node, float prob) {
    probability_accum += prob;
    
  }

  inline MarkovNode * next() {
   // float r = randomf() * probability_accum;
   
  }
  MarkovNode(){
    probability_accum = 0;
    //forwardConnections.push_back({this, 0.5});
  }
  MarkovNode(unsigned int d) : data(d) {}
};


class MarkovProcess {
private:
  inline float randu(){
    return 0.5;
  }

  std::vector<MarkovNode> nodes;
public:
	inline int step(int i){
    //
    return 0;
	}
	
	inline void setup(int p1){
    
	}

  MarkovProcess(){
    nodes.push_back(MarkovNode(5));
  }
} markov;
