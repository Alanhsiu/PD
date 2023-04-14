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
    void floorplan(double alpha);

    void writeResult(fstream& outfile);

   private:
    int _outlineWidth;
    int _outlineHeight;
    int _blkNum;
    int _tmlNum;
    int _netNum;
    vector<Net*> _netArray;
    vector<Block*> _blkArray;
    vector<Terminal*> _tmlArray;
    map<string, Block*> _blkMap;
    map<string, Terminal*> _tmlMap;

    int _fianlCost;
    int _wireLength;
    int _chipArea;
    int _chipWidth;
    int _chipHeight;

    void clear();
};

#endif