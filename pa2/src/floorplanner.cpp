#include "floorplanner.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <math.h>

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
    // init _nodeArray
    for (int i = 0; i < _blkNum; ++i) {
        _nodeArray.push_back(new Node(i));
        _blkArray[i]->setNode(_nodeArray[i]);
        _yContourWidth += _blkArray[i]->getWidth();
    }
    // To get the block by a node, use _blkArray[node->getId()].
    // To get the node by a block, use _blkArray[i]->getNode().

    // init B*-tree
    Node* dummyNode = new Node(-1);
    dummyNode->setLChild(_blkArray[0]->getNode());
    _blkArray[0]->getNode()->setParent(dummyNode);
    _root = dummyNode;
    buildBStarTree(0);

    // init _yContour
    _yContour.resize(_yContourWidth);

    return;
}

void Floorplanner::buildBStarTree(int start_idx) {
    for (int i = start_idx; i < _blkNum; ++i) {
        // set parent node
        if (i != 0)
            _nodeArray[i]->setParent(_nodeArray[(i - 1) / 2]);
        // set child node
        if (i % 2 == 1)
            _nodeArray[(i - 1) / 2]->setLChild(_nodeArray[i]);
        else
            _nodeArray[(i - 1) / 2]->setRChild(_nodeArray[i]);
    }
    printTree(_root->getLChild(), 0);
    return;
}

void Floorplanner::computeCoordinate(Node* node) {
    if (Node* lChild = node->getLChild()) {
        int width = _blkArray[lChild->getId()]->getWidth();
        int height = _blkArray[lChild->getId()]->getHeight();

        // set X coordinate, which is the parent node's X + parent node's width
        int startX = node->getId() == -1 ? 0 : (node->getX() + _blkArray[node->getId()]->getWidth());
        lChild->setX(startX);
        _chipWidth = std::max(_chipWidth, startX + width);

        // set Y coordinate and update _yContour
        int localMax = 0;
        for (int i = startX; i < startX + width; ++i)
            localMax = std::max(localMax, _yContour[i]);
        for (int i = startX; i < startX + width; ++i)
            _yContour[i] = localMax + height;
        lChild->setY(localMax);
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
        int localMax = 0;
        for (int i = startX; i < startX + width; ++i)
            localMax = std::max(localMax, _yContour[i]);
        for (int i = startX; i < startX + width; ++i)
            _yContour[i] = localMax + height;
        rChild->setY(localMax);
        _chipHeight = std::max(_chipHeight, localMax + height);

        computeCoordinate(rChild);
    }
}

void Floorplanner::computeWireLength() {
    _wireLength = 0;
    for (int i = 0; i < _netNum; ++i) {
        _netArray[i]->calcHPWL();
        _wireLength += _netArray[i]->getHPWL();
        // cout << "Net " << i << " : ";
        // for (int j = 0; j < _netArray[i]->getDegree(); ++j)
        //     cout << _netArray[i]->getTerm(j)->getName() << ":" << _netArray[i]->getTerm(j)->getX() << "," << _netArray[i]->getTerm(j)->getY() << " ";
        // cout << endl;
        // cout << "HPWL = " << _netArray[i]->getHPWL() << endl;
    }
}

double Floorplanner::computeCost() {
    // cout << "Wire length = " << _wireLength << endl;
    // cout << "Chip width = " << _chipWidth << endl;
    // cout << "Chip height = " << _chipHeight << endl;
    return _alpha * _chipHeight * _chipWidth + (1 - _alpha) * _wireLength;
}

double Floorplanner::reCompute() {
    clearYContour();
    _chipWidth = 0;
    _chipHeight = 0;
    computeCoordinate(_root);
    computeWireLength();
    return computeCost();
}

bool Floorplanner::checkValid() {
    if (_chipWidth > _outlineWidth || _chipHeight > _outlineHeight) {
        cout << "Error: chip size exceeds outline size" << endl;
        cout << "Chip width = " << _chipWidth << ", outline width = " << _outlineWidth << endl;
        cout << "Chip height = " << _chipHeight << ", outline height = " << _outlineHeight << endl;
        return false;
    }
    return true;
}

void Floorplanner::swapNodes(Node* node1, Node* node2) {
    Node* parent1 = node1->getParent();
    Node* parent2 = node2->getParent();
    int side1 = 0;
    int side2 = 0;
    parent1->getLChild() == node1 ? side1 = 0 : side1 = 1;
    parent2->getLChild() == node2 ? side2 = 0 : side2 = 1;
    side1 == 0 ? parent1->setLChild(node2) : parent1->setRChild(node2);
    side2 == 0 ? parent2->setLChild(node1) : parent2->setRChild(node1);
    node1->setParent(parent2);
    node2->setParent(parent1);

    Node* lchild1 = node1->getLChild();
    Node* rchild1 = node1->getRChild();
    Node* lchild2 = node2->getLChild();
    Node* rchild2 = node2->getRChild();
    node1->setLChild(lchild2);
    node1->setRChild(rchild2);
    node2->setLChild(lchild1);
    node2->setRChild(rchild1);
    if (lchild1 != NULL)
        lchild1->setParent(node2);
    if (rchild1 != NULL)
        rchild1->setParent(node2);
    if (lchild2 != NULL)
        lchild2->setParent(node1);
    if (rchild2 != NULL)
        rchild2->setParent(node1);
}

void Floorplanner::simulatedAnnealing() {
    // bool isValid = checkValid();
    double Temp = 1000;
    double ratio = 0.9;
    while (Temp > 1) {
        int op = rand() % 3;
        switch (op) {
            case 0: {  // rotate a macro
                cout << "case 0" << endl;
                int id = rand() % _blkNum;
                _blkArray[id]->rotate();
                double cost = reCompute();
                if (cost < _finalCost || exp((_finalCost - cost) / Temp) > (double)rand() / RAND_MAX) {
                    _finalCost = cost;
                    cout << "rotate " << _blkArray[id]->getName() << ", cost = " << cost << endl;
                    break;
                }

                // recover
                _blkArray[id]->rotate();
                reCompute();
                break;
            }
            case 1: {                        // delete and insert a macro
                cout << "case 1" << endl;    
                vector<int> leafOrUniaryId;  // tree nodes with no child or only one child
                vector<int> leafId;
                for (int i = 0; i < _blkNum; ++i)
                    if (_nodeArray[i]->getLChild() == NULL && _nodeArray[i]->getRChild() == NULL)
                        leafId.push_back(i);
                Node* node = _nodeArray[leafId[rand() % leafId.size()]];
                Node* parent = node->getParent();

                // delete node
                int side = 0;
                if (parent->getLChild() == node)
                    parent->setLChild(NULL);
                else {
                    parent->setRChild(NULL);
                    side = 1;
                }
                node->setParent(NULL);
                // insert node
                for (int i = 0; i < _blkNum; ++i)
                    if ((_blkArray[i]->getNode()->getLChild() == NULL ||_blkArray[i]->getNode()->getRChild() == NULL) && i!=node->getId())
                        leafOrUniaryId.push_back(i);
                    
                Node* targetNode = _nodeArray[leafOrUniaryId[rand() % leafOrUniaryId.size()]];
                int leftOrRight = rand() % 2;
                node->setParent(targetNode);
                if (leftOrRight == 0 && targetNode->getLChild() == NULL)
                    targetNode->setLChild(node);
                else
                    targetNode->setRChild(node);

                // recompute
                double cost = reCompute();
                if (cost < _finalCost || exp((_finalCost - cost) / Temp) > (double)rand() / RAND_MAX) {
                    _finalCost = cost;
                    break;
                }

                // recover
                if (leftOrRight == 0 && targetNode->getLChild() == node)
                    targetNode->setLChild(NULL);
                else
                    targetNode->setRChild(NULL);
                node->setParent(NULL);
                if (side == 0)
                    parent->setLChild(node);
                else
                    parent->setRChild(node);
                node->setParent(parent);
                reCompute();
                break;
            }
            case 2: {  // swap two nodes
                break;
                int id1 = rand() % _blkNum;
                int id2 = rand() % _blkNum;
                if (id1 == id2)
                    break;
                cout << "swap(before) " << _blkArray[id1]->getName() << " and " << _blkArray[id2]->getName() << endl;
                Node* node1 = _nodeArray[id1];
                Node* node2 = _nodeArray[id2];
                Node* parent1 = node1->getParent();
                Node* parent2 = node2->getParent();
                if (parent1 == node2 || parent2 == node1)
                    break;
                swapNodes(node1, node2);

                // recompute
                double cost = reCompute();
                cout << "cost = " << cost << endl;
                if (cost < _finalCost || exp((_finalCost - cost) / Temp) > (double)rand() / RAND_MAX) {
                    _finalCost = cost;
                    cout << "swap(after) " << _blkArray[id1]->getName() << " and " << _blkArray[id2]->getName() << ", and the cost is " << cost << endl;
                    printTree(_root->getLChild(), 0);
                    break;
                }
                // recover
                swapNodes(node2, node1);
                cout << "swap(restore) " << _blkArray[id1]->getName() << " and " << _blkArray[id2]->getName() << endl;
                reCompute();
                break;
            }
        }
        Temp *= ratio;
    }
    // int id = rand() % _blkNum;
    // cout << "rotate " << _blkArray[id]->getName() << endl;
    // _blkArray[id]->rotate();
    // computeCoordinate(_root);
    // computeWireLength();
    // computeCost();
}

void Floorplanner::floorplan(double alpha) {
    _alpha = alpha;
    initialPlacement();
    _chipWidth = 0;
    _chipHeight = 0;
    computeCoordinate(_root->getLChild());
    computeWireLength();
    _finalCost = computeCost();

    simulatedAnnealing();

    printTree(_root->getLChild(), 0);
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
        // cout << _blkArray[i]->getName() << " " << _blkArray[i]->getNode()->getX() << " " << _blkArray[i]->getNode()->getY() << endl;
        int x1 = _blkArray[i]->getNode()->getX();
        int y1 = _blkArray[i]->getNode()->getY();
        int x2 = x1 + _blkArray[i]->getWidth();
        int y2 = y1 + _blkArray[i]->getHeight();
        cout << _blkArray[i]->getName() << " (" << x1 << "," << y1 << ") (" << x2 << "," << y2 << ")" << endl;
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
    cout << _yContour.size() << ':' << _yContour[_yContour.size() - 1] << endl;
    cout << "===================================================" << endl;
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

void Floorplanner::clearYContour() {
    _yContour.clear();

    // init _yContour
    _yContour.resize(_yContourWidth);
}

void Floorplanner::clearBStarTree(Node* node) {
    if (node == NULL)
        return;
    clearBStarTree(node->getLChild());
    clearBStarTree(node->getRChild());
    node->setParent(NULL);
    node->setLChild(NULL);
    node->setRChild(NULL);
}

void Floorplanner::clear() {
    for (int i = 0; i < _blkNum; ++i)
        delete _blkArray[i];
    for (int i = 0; i < _tmlNum; ++i)
        delete _tmlArray[i];
    for (int i = 0; i < _netNum; ++i)
        delete _netArray[i];
    clearYContour();
    clearBStarTree(_root->getLChild());
    return;
}
