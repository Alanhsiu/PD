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
    void computeXCoordinate(Node* root);
    void computeYCoordinate(Node* root);
    void updateYContour(int startX, int endX, int height);
    void floorplan(double alpha);

    void printSummary() const;
    void writeResult(fstream& outfile);

   private:
    int                     _outlineWidth;
    int                     _outlineHeight;
    int                     _blkNum;
    int                     _tmlNum;
    int                     _netNum;
    Node*                   _root;
    vector<int>             _yContour;
    vector<Net*>            _netArray;
    vector<Node*>           _nodeArray;
    vector<Block*>          _blkArray;
    vector<Terminal*>       _tmlArray;
    map<string, Block*>     _blkMap;
    map<string, Terminal*>  _tmlMap;

    int                     _finalCost;
    int                     _wireLength;
    int                     _chipArea;
    int                     _chipWidth;
    int                     _chipHeight;

    void clear();
};

#endif