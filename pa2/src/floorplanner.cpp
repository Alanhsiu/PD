#include "floorplanner.h"
#include <algorithm>
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
    // sort(_blkArray.begin(), _blkArray.end(), [](Block* a, Block* b) {
    //     return a->getArea() > b->getArea();
    // });
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
        net->setDegree(degree);
        for (int j = 0; j < degree; ++j) {
            string name;
            inFile >> name;
            if (_blkMap.count(name) != 0)  // if name is a block
                net->addTerm(_blkMap[name]);
            else  // if name is a terminal
                net->addTerm(_tmlMap[name]);
        }
        _netArray.push_back(net);
    }
    return;
}

void Floorplanner::initialPlacement() {
    for (int i = 0; i < _blkNum; ++i) {
        _nodeArray.push_back(new Node(i));
        _blkArray[i]->setNode(_nodeArray[i]);
    }

    _root = _nodeArray[0];
    _root->setX(0);
    _root->setY(0);
    _root->setParent(NULL);

    for (int i = 1; i < _blkNum; ++i) {
        // set parent node
        _nodeArray[i]->setParent(_nodeArray[(i - 1) / 2]);
        // set child node
        if (i % 2 == 1)
            _nodeArray[(i - 1) / 2]->setLChild(_nodeArray[i]);
        else
            _nodeArray[(i - 1) / 2]->setRChild(_nodeArray[i]);
    }

    // init _yContour
    _yContour.resize(_outlineWidth);
    int width = _blkArray[_root->getId()]->getWidth();
    int height = _blkArray[_root->getId()]->getHeight();
    for (int i = 0; i < width; ++i)
        _yContour[i] = height;

    return;
}

void Floorplanner::insertNode(Node* node) {
}

void Floorplanner::computeCoordinate(Node* node) {
    printYContour();

    if (Node* lChild = node->getLChild()) {
        int width = _blkArray[lChild->getId()]->getWidth();
        int height = _blkArray[lChild->getId()]->getHeight();

        // set X coordinate, which is the parent node's X + parent node's width
        int startX = node->getX() + _blkArray[node->getId()]->getWidth();
        lChild->setX(startX);
        _chipWidth = std::max(_chipWidth, startX + width);

        // set Y coordinate and update _yContour
        auto start_it = _yContour.begin() + startX;
        auto end_it = start_it + width;
        auto it = std::max_element(start_it, end_it);
        int localMax = *it;
        lChild->setY(localMax);
        std::fill(start_it, end_it, localMax + height);
        _chipHeight = std::max(_chipHeight, localMax + height);

        computeCoordinate(lChild);
    }
    if (Node* rChild = node->getRChild()) {
        int width = _blkArray[rChild->getId()]->getWidth();
        int height = _blkArray[rChild->getId()]->getHeight();

        // set X coordinate, which is the same as parent node
        int startX = node->getX();
        rChild->setX(startX);
        _chipWidth = std::max(_chipWidth, startX + width);

        // set Y coordinate and update _yContour
        auto start_it = _yContour.begin() + startX;
        auto end_it = start_it + width;
        auto it = std::max_element(start_it, end_it);
        int y = std::max(*it, node->getY());
        std::fill(start_it, end_it, y + height);
        rChild->setY(y);
        _chipHeight = std::max(_chipHeight, y + height);

        computeCoordinate(rChild);
    }
}

void Floorplanner::computeWireLength() {
    _wireLength = 0;
    for (int i = 0; i < _netNum; ++i) {
        _netArray[i]->calcHPWL();
        _wireLength += _netArray[i]->getHPWL();
        cout << "Net " << i << " : ";
        for (int j = 0; j < _netArray[i]->getDegree(); ++j) {
            cout << _netArray[i]->getTerm(j)->getName() << ":" << _netArray[i]->getTerm(j)->getX() << "," << _netArray[i]->getTerm(j)->getY() << " ";
        }
        cout << endl;
        cout << "HPWL = " << _netArray[i]->getHPWL() << endl;
    }
    cout << "Total wire length = " << _wireLength << endl;
}

void Floorplanner::computeCost() {
    _finalCost = _alpha * _chipHeight * _chipWidth + (1 - _alpha) * _wireLength;
}

bool Floorplanner::checkValid() {
    if(_chipWidth > _outlineWidth || _chipHeight > _outlineHeight){
        cout << "Error: chip size exceeds outline size" << endl;
        cout << "Chip width = " << _chipWidth << ", outline width = " << _outlineWidth << endl;
        cout << "Chip height = " << _chipHeight << ", outline height = " << _outlineHeight << endl;
        return false;
    }
    return true;
}

void Floorplanner::updateYContour(int startX, int endX, int height) {
    // TODO
}

void Floorplanner::floorplan(double alpha) {
    _alpha = alpha;
    initialPlacement();
    computeCoordinate(_root);
    computeWireLength();
    computeCost();
    checkValid();
    printTree(_root, 0);
    printSummary();
    printCoordinate();
}

void Floorplanner::printTree(Node* node, int level) const {
    if (level == 0)
        cout << "==================== Tree ====================" << endl;
    if (node == NULL)
        return;
    printTree(node->getRChild(), level + 1);
    for (int i = 0; i < level; ++i)
        cout << "    ";
    cout << _blkArray[node->getId()]->getName() << endl;
    printTree(node->getLChild(), level + 1);
}

void Floorplanner::printCoordinate() const {
    cout << "==================== Coordinate ====================" << endl;
    for (int i = 0; i < _blkNum; ++i) {
        cout << _blkArray[i]->getName() << " " << _blkArray[i]->getNode()->getX() << " " << _blkArray[i]->getNode()->getY() << endl;
    }
}

void Floorplanner::printYContour() const {
    cout << "==================== Y Contour ====================" << endl;
    int i = 0;
    int height = 0;
    while (i < _yContour.size()) {
        if (_yContour[i] != height) {
            height = _yContour[i];
            cout << i << ':' << height << endl;
        }
        ++i;
    }
}

void Floorplanner::printSummary() const {
    cout << "==================== Summary ====================" << endl;
    cout << "Final Cost = " << _finalCost << endl;
    cout << "Wire Length = " << _wireLength << endl;
    cout << "Chip Area = " << _chipWidth * _chipHeight << endl;
    cout << "Chip Width = " << _chipWidth << endl;
    cout << "Chip Height = " << _chipHeight << endl;
    cout << "Elapsed Time = " << (double)clock() / CLOCKS_PER_SEC << endl;
}

void Floorplanner::writeResult(fstream& outFile) {
    outFile << _finalCost << '\n';
    outFile << _wireLength << '\n';
    outFile << _chipWidth * _chipHeight << '\n';
    outFile << _chipWidth << ' ' << _chipHeight << '\n';
    outFile << (double)clock() / CLOCKS_PER_SEC << '\n';
    for (int i = 0; i < _blkNum; ++i) {
        outFile << _blkArray[i]->getName() << ' ' << _blkArray[i]->getNode()->getX() << ' ' << _blkArray[i]->getNode()->getY() << ' ' << _blkArray[i]->getNode()->getX() + _blkArray[i]->getWidth() << ' ' << _blkArray[i]->getNode()->getY() + _blkArray[i]->getHeight() << '\n';
    }
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
