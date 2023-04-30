#include "floorplanner.h"
#include <math.h>
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
        _nodeArray[i]->setArea(_blkArray[i]->getArea());
        _yContourWidth += std::max(_blkArray[i]->getWidth(), _blkArray[i]->getHeight());
    }
    sort(_nodeArray.begin(), _nodeArray.end(), [](Node* a, Node* b) {
        return a->getArea() > b->getArea();
    });

    // init B*-tree
    Node* dummyNode = new Node(-1);
    dummyNode->setLChild(_nodeArray[0]);
    _nodeArray[0]->setParent(dummyNode);
    _nodeArray[0]->setSide(0);

    _root = dummyNode;
    buildBStarTree(1);

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
        if (i % 2 == 1) {
            _nodeArray[(i - 1) / 2]->setLChild(_nodeArray[i]);
            _nodeArray[i]->setSide(0);
        } else {
            _nodeArray[(i - 1) / 2]->setRChild(_nodeArray[i]);
            _nodeArray[i]->setSide(1);
        }
    }
    printTree(_root, -1);
    return;
}

void Floorplanner::computeCoordinate(Node* node) {
    if (Node* lChild = node->getLChild()) {
        // cout << "lChild: " << lChild->getId() << ", and its parent: " << lChild->getParent()->getId() << endl;
        // cout << "lChild: " << _blkArray[lChild->getId()]->getName() << ", and its parent: " << _blkArray[lChild->getParent()->getId()]->getName() << endl;
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

        // printYContour();

        computeCoordinate(lChild);
    }
    if (Node* rChild = node->getRChild()) {
        // cout << "rChild: " << rChild->getId() << ", and its parent: " << rChild->getParent()->getId() << endl;
        // cout << "rChild: " << _blkArray[rChild->getId()]->getName() << ", and its parent: " << _blkArray[rChild->getParent()->getId()]->getName() << endl;
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

        // printYContour();

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
    int penalty = 0;
    if (_chipWidth > _outlineWidth)
        penalty += _chipWidth - _outlineWidth;
    if (_chipHeight > _outlineHeight)
        penalty += _chipHeight - _outlineHeight;
    return _alpha * _chipHeight * _chipWidth + (1 - _alpha) * _wireLength + 10000 * penalty;
    // return penalty;
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

void Floorplanner::DFS(Node* node, vector<Node*>& nodeArray) {
    if (node == NULL)
        return;
    DFS(node->getLChild(), nodeArray);
    DFS(node->getRChild(), nodeArray);
    if (node->getLChild() == NULL || node->getRChild() == NULL)
        nodeArray.push_back(node);
}

void Floorplanner::swapNodes(Node* node1, Node* node2) {
    // exchange parent
    Node* parent1 = node1->getParent();
    Node* parent2 = node2->getParent();
    int side1 = node1->getSide();
    int side2 = node2->getSide();
    // parent1->getLChild() == node1 ? side1 = 0 : side1 = 1;
    // parent2->getLChild() == node2 ? side2 = 0 : side2 = 1;
    // side1 == 0 ? parent1->setLChild(node2) : parent1->setRChild(node2);
    // side2 == 0 ? parent2->setLChild(node1) : parent2->setRChild(node1);
    if (side1 == 0) {
        parent1->setLChild(node2);
        node2->setSide(0);
    } else {
        parent1->setRChild(node2);
        node2->setSide(1);
    }
    if (side2 == 0) {
        parent2->setLChild(node1);
        node1->setSide(0);
    } else {
        parent2->setRChild(node1);
        node1->setSide(1);
    }
    node1->setParent(parent2);
    node2->setParent(parent1);

    // exchange children
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
    double Temp = 10000;
    double ratio = 0.9;
    while (Temp > 1) {
        int iter = 500;
        if (Temp > 100)
            iter = 500;
        else if (Temp > 30)
            iter = 1000;
        else
            iter = 500;
        for (int i = 0; i < 10; ++i) {
            int op = rand() % 5;
            switch (op) {
                case 0: {  // rotate a macro
                    break;
                    cout << "case 0: original cost is " << _finalCost << endl;
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
                case 1: {  // delete and insert a macro
                    break;
                    cout << "case 1: original cost is " << _finalCost << endl;
                    printTree(_root, -1);
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
                        if ((_nodeArray[i]->getLChild() == NULL || _nodeArray[i]->getRChild() == NULL) && _nodeArray[i] != node)
                            leafOrUniaryId.push_back(i);

                    Node* targetNode = _nodeArray[leafOrUniaryId[rand() % leafOrUniaryId.size()]];
                    int leftOrRight = rand() % 2;
                    node->setParent(targetNode);
                    if (targetNode->getLChild() == NULL && targetNode->getRChild() == NULL && leftOrRight == 0)
                        targetNode->setLChild(node);
                    else if (targetNode->getLChild() == NULL && targetNode->getRChild() == NULL && leftOrRight == 1)
                        targetNode->setRChild(node);
                    else if (targetNode->getLChild() == NULL) {
                        leftOrRight = 0;
                        targetNode->setLChild(node);
                    } else {
                        leftOrRight = 1;
                        targetNode->setRChild(node);
                    }

                    // recompute
                    double cost = reCompute();
                    if (cost < _finalCost || exp((_finalCost - cost) / Temp) > (double)rand() / RAND_MAX) {
                        // printTree(_root->getLChild(), 0);
                        cout << "delete " << _blkArray[node->getId()]->getName() << " and insert to " << _blkArray[targetNode->getId()]->getName() << ", cost = " << cost << endl;
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
                    cout << "case 2: original cost is " << _finalCost << endl;
                    int id1 = rand() % _blkNum;
                    int id2 = rand() % _blkNum;
                    if (id1 == id2)
                        break;
                    cout << "swap(before) " << _blkArray[id1]->getName() << " and " << _blkArray[id2]->getName() << endl;
                    Node* node1 = _nodeArray[id1];
                    Node* node2 = _nodeArray[id2];
                    Node* parent1 = node1->getParent();
                    Node* parent2 = node2->getParent();
                    // cout<< "parent1 = " << _blkArray[parent1->getId()]->getName() << endl;
                    // cout<< "parent2 = " << _blkArray[parent2->getId()]->getName() << endl;
                    cout << "node1 = " << node1->getId() << endl;
                    cout << "node2 = " << node2->getId() << endl;
                    cout << "parent1 = " << parent1->getId() << endl;
                    cout << "parent2 = " << parent2->getId() << endl;
                    if (parent1 == node2 || parent2 == node1 || parent1 == parent2)
                        break;
                    swapNodes(node1, node2);

                    // recompute
                    double cost = reCompute();
                    cout << "cost = " << cost << endl;
                    if (cost < _finalCost || exp((_finalCost - cost) / Temp) > (double)rand() / RAND_MAX) {
                        _finalCost = cost;
                        cout << "swap(after) " << _blkArray[id1]->getName() << " and " << _blkArray[id2]->getName() << ", and the cost is " << cost << endl;
                        printTree(_root, -1);
                        break;
                    }
                    // recover
                    swapNodes(node1, node2);
                    cout << "swap(restore) " << _blkArray[id1]->getName() << " and " << _blkArray[id2]->getName() << endl;
                    printTree(_root, -1);
                    reCompute();
                    break;
                }
                case 3: {  // swap sibling
                    break;
                    cout << "case 3: original cost is " << _finalCost << endl;
                    int id = rand() % _blkNum;
                    Node* node = _nodeArray[id];
                    if (node->getLChild() == NULL || node->getRChild() == NULL)
                        break;
                    Node* lChild = node->getLChild();
                    Node* rChild = node->getRChild();
                    cout << "swap(before) " << _blkArray[lChild->getId()]->getName() << " and " << _blkArray[rChild->getId()]->getName() << endl;
                    swapNodes(lChild, rChild);
                    cout << "hi" << endl;

                    // recompute
                    double cost = reCompute();
                    if (cost < _finalCost || exp((_finalCost - cost) / Temp) > (double)rand() / RAND_MAX) {
                        _finalCost = cost;
                        cout << "swap(after) " << _blkArray[lChild->getId()]->getName() << " and " << _blkArray[rChild->getId()]->getName() << ", and the cost is " << cost << endl;
                        break;
                    }
                    // recover
                    swapNodes(lChild, rChild);
                    reCompute();
                    cout << "recover" << endl;

                    break;
                }
                case 4: {  // delete and insert a macro in anyplace
                    // break;
                    printTree(_root, -1);
                    cout << "case 4: original cost is " << _finalCost << endl;
                    int id = rand() % _blkNum;
                    Node* node = _nodeArray[id];
                    Node* parent = node->getParent();
                    if (parent == NULL || parent->getId() == -1)
                        break;
                    cout << "delete " << _blkArray[node->getId()]->getName() << endl;
                    int side = _nodeArray[id]->getSide();
                    cout << "side = " << side << endl;
                    (side == 0) ? parent->setLChild(NULL) : parent->setRChild(NULL);
                    node->setParent(NULL);

                    vector<Node*> remain;
                    DFS(_root->getLChild(), remain);
                    int target = rand() % remain.size();
                    Node* targetNode = remain[target];
                    for(int i = 0; i < remain.size(); i++)
                        cout << _blkArray[remain[i]->getId()]->getName() << " ";
                    cout << endl;
                    cout << "insert " << _blkArray[node->getId()]->getName() << " to " << _blkArray[targetNode->getId()]->getName() << endl;
                    int leftOrRight = rand() % 2;
                    node->setParent(targetNode);
                    if (targetNode->getLChild() == NULL && targetNode->getRChild() == NULL && leftOrRight == 0) {
                        targetNode->setLChild(node);
                        node->setSide(0);
                    } else if (targetNode->getLChild() == NULL && targetNode->getRChild() == NULL && leftOrRight == 1) {
                        targetNode->setRChild(node);
                        node->setSide(1);
                    } else if (targetNode->getLChild() == NULL) {
                        leftOrRight = 0;
                        targetNode->setLChild(node);
                        node->setSide(0);
                    } else {
                        leftOrRight = 1;
                        targetNode->setRChild(node);
                        node->setSide(1);
                    }
                    cout << "insert done" << endl;
                    printTree(_root, -1);

                    // recompute
                    double cost = reCompute();
                    cout << "cost = " << cost << endl;
                    if (cost < _finalCost || exp((_finalCost - cost) / Temp) > (double)rand() / RAND_MAX) {
                        _finalCost = cost;
                        cout << "delete(after) " << _blkArray[node->getId()]->getName() << ", and the cost is " << cost << endl;
                        break;
                    }
                    // recover
                    if (leftOrRight == 0 && targetNode->getLChild() == node)
                        targetNode->setLChild(NULL);
                    else
                        targetNode->setRChild(NULL);

                    if (side == 0) {
                        parent->setLChild(node);
                        node->setSide(0);
                    } else {
                        parent->setRChild(node);
                        node->setSide(1);
                    }
                    node->setParent(parent);
                    reCompute();
                    cout << "recover" << endl;
                    break;
                }
            }
        }
        Temp *= ratio;
    }
}

void Floorplanner::floorplan(double alpha) {
    _alpha = alpha;
    initialPlacement();
    _chipWidth = 0;
    _chipHeight = 0;
    computeCoordinate(_root);
    computeWireLength();
    _finalCost = computeCost();

    simulatedAnnealing();

    printTree(_root, -1);
    printSummary();
    printCoordinate();
}

void Floorplanner::printTree(Node* node, int level) const {
    if (level == -1) {
        cout << "==================== Tree ====================" << endl;
        // cout << "parent: ";
        // for (int i = 0; i < _blkNum; ++i)
        //     cout << "node" << _nodeArray[i]->getId() << " 's parent is " << _nodeArray[i]->getParent()->getId() << ", ";
        // cout << endl;
        // for (int i = 0; i < _blkNum; ++i)
        //     cout << _blkArray[i]->getName() << " 's parent is " << _blkArray[i]->getNode()->getParent()->getId() << ", ";
        // cout << endl;
    }
    if (node == NULL)
        return;
    printTree(node->getRChild(), level + 1);
    for (int i = 0; i < level; ++i)
        cout << "    ";
    if (level != -1)
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
    cout << "Final Cost = " << _alpha * _chipHeight * _chipWidth + (1 - _alpha) * _wireLength << endl;
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
