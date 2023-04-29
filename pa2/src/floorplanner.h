#ifndef FLOORPANNER_H
#define FLOORPANNER_H

#include <fstream>
#include <map>
#include <vector>
#include "module.h"
using namespace std;

class Floorplanner {
   public:
    Floorplanner(fstream& input_blk, fstream& input_net) {
        parseInput_blk(input_blk);
        parseInput_net(input_net);
    }
    ~Floorplanner() {
        clear();
    }

    void parseInput_blk(fstream& input_blk);
    void parseInput_net(fstream& input_net);

    void initialPlacement();
    void buildBStarTree(int start_idx);
    void computeCoordinate(Node* node);
    void computeWireLength();
    double computeCost();
    double reCompute();
    bool checkValid();
    void swapNodes(Node* node1, Node* node2);
    void simulatedAnnealing();
    void floorplan(double alpha);
    void clearYContour();
    void clearBStarTree(Node* node);

    void printTree(Node* root, int level) const;
    void printCoordinate() const;
    void printYContour() const;
    void printSummary() const;
    void writeResult(fstream& outfile);

   private:
    int _outlineWidth;
    int _outlineHeight;
    int _yContourWidth;
    int _blkNum;
    int _tmlNum;
    int _netNum;
    double _alpha;
    Node* _root;  // B*-tree root
    vector<int> _yContour;
    vector<Net*> _netArray;
    vector<Node*> _nodeArray;
    vector<Block*> _blkArray;
    vector<Terminal*> _tmlArray;
    map<string, Block*> _blkMap;
    map<string, Terminal*> _tmlMap;

    int _finalCost;
    float _wireLength;
    int _chipWidth;
    int _chipHeight;

    void clear();
};

#endif