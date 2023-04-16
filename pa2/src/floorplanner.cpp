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
    sort(_blkArray.begin(), _blkArray.end(), [](Block* a, Block* b) {
        return a->getArea() > b->getArea();
    });
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

void Floorplanner::initialPlacement() {
    for (int i = 0; i < _blkNum; ++i)
        _nodeArray.push_back(new Node(i));

    _root = _nodeArray[0];
    _root->setLChild(_nodeArray[1]);
    _root->setRChild(_nodeArray[2]);
    for (int i = 1; i < _blkNum; ++i) {
        // set parent node
        _nodeArray[i]->setParent(_nodeArray[(i - 1) / 2]);
        // set child node
        if (i % 2 == 1)
            _nodeArray[(i - 1) / 2]->setLChild(_nodeArray[i]);
        else
            _nodeArray[(i - 1) / 2]->setRChild(_nodeArray[i]);
    }
    for (int i = 0; i < _blkNum; ++i)
        cout << _nodeArray[i]->getId() << " ";
    cout << endl;
    return;
}

void Floorplanner::computeXCoordinate(Node* node) {
    int x=0;
    Node* temp = _root;
    while (temp->getLChild() != NULL) {
        temp = temp->getLChild();
        x += _blkArray[temp->getId()]->getWidth();
    }
    cout << "x = " << x << endl;
}

void Floorplanner::computeYCoordinate(Node* node) {
    int y=0;
    Node* temp = _root;
    while (temp->getLChild() != NULL) {
        temp = temp->getLChild();
        y += _blkArray[temp->getId()]->getWidth();
    }
    cout << "y = " << y << endl;
}

void Floorplanner::updateYContour(int startX, int endX, int height) {
    // TODO
}

void Floorplanner::floorplan(double alpha) {
    // TODO
    initialPlacement();
    printSummary();
}

void Floorplanner::printSummary() const {
    // TODO
    cout << "Final Cost = " << _finalCost << endl;
    cout << "Wire Length = " << _wireLength << endl;
    cout << "Chip Area = " << _chipArea << endl;
    cout << "Chip Width = " << _chipWidth << endl;
    cout << "Chip Height = " << _chipHeight << endl;
    cout << "Elapsed Time = " << (double)clock() / CLOCKS_PER_SEC << endl;
}

void Floorplanner::writeResult(fstream& outFile) {
    outFile << _finalCost << '\n';
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
