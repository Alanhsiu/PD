#include "floorplanner.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

void Floorplanner::parseInput_blk(fstream& inFile) {
    string str;
    inFile >> str;
    inFile >> str;
    _outlineWidth = std::stoi(str);
    inFile >> str;
    _outlineHeight = std::stoi(str);
    inFile >> str;
    inFile >> str;
    _blkNum = std::stoi(str);
    inFile >> str;
    inFile >> str;
    _tmlNum = std::stoi(str);

    for (int i = 0; i < _blkNum; ++i) {
        string str, name;
        inFile >> name;
        inFile >> str;
        int width = std::stoi(str);
        inFile >> str;
        int height = std::stoi(str);
        Block* block = new Block(name, width, height);
        _blkArray.push_back(block);
        _blkMap[name] = block;
    }
    for (int i = 0; i < _tmlNum; ++i) {
        string str, name;
        inFile >> name;
        inFile >> str;
        inFile >> str;
        int x = std::stoi(str);
        inFile >> str;
        int y = std::stoi(str);
        Terminal* terminal = new Terminal(name, x, y);
        _tmlArray.push_back(terminal);
        _tmlMap[name] = terminal;
    }
    return;
}

void Floorplanner::parseInput_net(fstream& inFile) {
    string str;
    inFile >> str;
    inFile >> str;
    _netNum = std::stoi(str);
    for (int i = 0; i < _netNum; ++i) {
        inFile >> str;
        inFile >> str;
        int degree = std::stoi(str);
        Net* net = new Net();
        for (int j = 0; j < degree; ++j) {
            string name;
            inFile >> name;
            if (_blkMap.count(name) != 0)
                net->addTerm(_blkMap[name]);
            else
                net->addTerm(_tmlMap[name]);
        }
        _netArray.push_back(net);
    }
    return;
}

void Floorplanner::floorplan(double alpha) {
    // TODO
    cout << "alpha = " << alpha << endl;
}

void Floorplanner::writeResult(fstream& outFile) {
    outFile << _fianlCost << '\n';
    outFile << _wireLength << '\n';
    outFile << _chipArea << '\n';
    outFile << _chipWidth << ' ' << _chipHeight << '\n';
    outFile << std::fixed << std::setprecision(2) << (double)clock() / CLOCKS_PER_SEC << '\n';
}
void Floorplanner::clear() {
    for (int i = 0; i < _blkNum; ++i)
        delete _blkArray[i];
    for (int i = 0; i < _tmlNum; ++i)
        delete _tmlArray[i];
    for (int i = 0; i < _netNum; ++i)
        delete _netArray[i];
    return;
}
