#include "partitioner.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include "cell.h"
#include "net.h"
using namespace std;

void Partitioner::parseInput(fstream& inFile) {
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);

    // Set up whole circuit
    while (inFile >> str) {
        if (str == "NET") {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net(netName));
            _netName2Id[netName] = netId;
            while (inFile >> cellName) {
                if (cellName == ";") {
                    tmpCellName = "";
                    break;
                } else {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0) {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId);
                        _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                        ++_cellNum;
                        tmpCellName = cellName;
                    }
                    // an existed cell
                    else {
                        if (cellName != tmpCellName) {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId);
                            _cellArray[cellId]->incPinNum();
                            _netArray[netId]->addCell(cellId);
                            tmpCellName = cellName;
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

void Partitioner::partition() {
    _loopNum = 30;
    _prevCutSize = -1;

    initPart();
    loop();
    for (int i = 0; i < _loopNum; ++i) {
        _earlyStopFlag = _loopNum - i + 3;
        if(_initCutSize > 150000)
            _earlyStopFlag = _loopNum - i;
        reset();
        if (_cutSize == _prevCutSize)
            break;
        _prevCutSize = _cutSize;
        loop();
    }
}

void Partitioner::loop() {
    initGain();
    initBList();
    while (findMaxGainNode())
        move();
    findBest();
}

void Partitioner::reset() {
    _initCutSize = _cutSize;
    _accGain = 0;
    _maxAccGain = 0;
    _moveStack.clear();
    _moveNum = 0;
    _bestMoveNum = 0;
    _earlyStopCount = 0;
    ResetCell();
    clearBList();
}

void Partitioner::initPart() {
    int halfCellNum = _cellNum / 2;
    _minCellNum = ceil(_cellNum * (1 - _bFactor) / 2);
    _maxCellNum = floor(_cellNum * (1 + _bFactor) / 2);
    _maxPinNum = 0;

    // init cell part and net info
    for (int i = 0; i < _cellNum; ++i) {
        Cell* cell = _cellArray[i];
        if (i < halfCellNum) {  // put half of the cells in part A
            cell->setPart(0);
            // update net info
            vector<int> netList = cell->getNetList();
            for (auto netId : netList) {
                _netArray[netId]->incPartCount(0);
            }
        } else {  // put the other half of the cells in part B
            cell->setPart(1);
            // update net info
            vector<int> netList = cell->getNetList();
            for (auto netId : netList) {
                _netArray[netId]->incPartCount(1);
            }
        }
        if (cell->getPinNum() > _maxPinNum)
            _maxPinNum = cell->getPinNum();
    }
    // init partSize
    _partSize[0] = halfCellNum;
    _partSize[1] = _cellNum - halfCellNum;

    for (int i = 0; i < _netNum; ++i)
        if (_netArray[i]->getPartCount(0) && _netArray[i]->getPartCount(1))
            ++_cutSize;
    // for (int i = 0; i < _cellNum; i++)
    //     _cellArray[i]->unlock();
    _initCutSize = _cutSize;
}

void Partitioner::initGain() {
    for (int i = 0; i < _cellNum; ++i) {
        Cell* cell = _cellArray[i];
        vector<int> netList = cell->getNetList();
        int netListSize = netList.size();
        for (int j = 0; j < netListSize; ++j) {
            int netId = netList[j];
            Net* net = _netArray[netId];
            int Fn = (cell->getPart()) ? net->getPartCount(1) : net->getPartCount(0);
            int Tn = (cell->getPart()) ? net->getPartCount(0) : net->getPartCount(1);

            if (Fn == 1)
                cell->incGain();
            if (Tn == 0)
                cell->decGain();
        }
    }
}

void Partitioner::initBList() {
    for (int i = 0; i < _cellNum; ++i)
        insertNode(_cellArray[i]->getNode(), _cellArray[i]->getPart());
}

void Partitioner::ResetCell() {
    for (int i = 0; i < _cellNum; ++i) {
        _cellArray[i]->setGain(0);
        _cellArray[i]->unlock();
    }
}

void Partitioner::clearBList() {
    for (int i = 0; i < 2; ++i)
        _bList[i].clear();
    for (int i = 0; i < _cellNum; ++i) {
        _cellArray[i]->getNode()->setNext(NULL);
        _cellArray[i]->getNode()->setPrev(NULL);
    }
}

bool Partitioner::findMaxGainNode() {
    // decide which part to move
    bool ableToMove0 = (_bList[0].empty() || _partSize[0] == _minCellNum) ? false : true;
    bool ableToMove1 = (_bList[1].empty() || _partSize[1] == _minCellNum) ? false : true;
    if (!ableToMove0 && !ableToMove1)
        return false;
    if (!ableToMove0)
        _movePart = 1;
    else if (!ableToMove1)
        _movePart = 0;
    else if (_bList[0].rbegin()->first < _bList[1].rbegin()->first)
        _movePart = 1;
    else
        _movePart = 0;
    if (_bList[_movePart].rbegin()->first < 0)
        ++_earlyStopCount;
    else
        _earlyStopCount = 0;
    if (_earlyStopCount > _earlyStopFlag)
        return false;

    _maxGainNode = _bList[_movePart].rbegin()->second;
    return true;
}

void Partitioner::move() {
    // move the cell with max gain in movePart
    Cell* moveCell = _cellArray[_maxGainNode->getId()];
    int F = _movePart;
    int T = 1 - F;
    moveCell->move();
    moveCell->lock();
    removeNode(_maxGainNode, F);
    ++_moveNum;
    ++_partSize[T];
    --_partSize[F];
    _moveStack.push_back(_maxGainNode->getId());
    _accGain += moveCell->getGain();
    if (_accGain > _maxAccGain) {
        _maxAccGain = _accGain;
        _bestMoveNum = _moveNum;
    }
    assert(_cutSize >= 0);

    // update gain
    vector<int> netList = moveCell->getNetList();
    for (auto netId : netList) {
        Net* net = _netArray[netId];
        net->incPartCount(T);
        net->decPartCount(F);
        if (net->getLock())
            continue;
        int afterF = net->getPartCount(F);
        int afterT = net->getPartCount(T);
        if (afterF != 0 && afterF != 1 && afterT != 1 && afterT != 2)
            continue;
        vector<int> cellList = net->getCellList();

        for (auto cellId : cellList) {
            Cell* cell = _cellArray[cellId];
            if (cell->getLock())
                continue;
            int gainChange = 0;
            int cellPart = cell->getPart();
            if (cellPart == F) {
                if (afterT == 1)
                    ++gainChange;
                if (afterF == 1)
                    ++gainChange;
            } else if (cellPart == T) {
                if (afterT == 2)
                    --gainChange;
                if (afterF == 0)
                    --gainChange;
            }
            if (gainChange != 0) {
                removeNode(cell->getNode(), cellPart);
                cell->changeGain(gainChange);
                insertNode(cell->getNode(), cellPart);
            }
        }
    }
}

void Partitioner::insertNode(Node* curNode, int part) {
    Cell* cell = _cellArray[curNode->getId()];
    int gain = cell->getGain();
    auto iter = _bList[part].find(gain);
    if (iter == _bList[part].end()) {  // if the gain is not in the map
        _bList[part][gain] = curNode;
    } else if (iter != _bList[part].end()) {  // if the gain is in the map
        Node* firstNode = iter->second;
        curNode->setNext(firstNode);
        curNode->setPrev(NULL);
        firstNode->setPrev(curNode);
        _bList[part][gain] = curNode;
    }
}

void Partitioner::removeNode(Node* curNode, int part) {
    Cell* cell = _cellArray[curNode->getId()];
    bool isHead = (curNode->getPrev() == NULL) ? true : false;
    bool isTail = (curNode->getNext() == NULL) ? true : false;
    if (isHead && isTail) {  // if the cell is the only one in the list
        _bList[part].erase(cell->getGain());
    } else if (isHead && !isTail) {  // if the cell is the head of the list
        Node* nextNode = curNode->getNext();
        nextNode->setPrev(NULL);
        curNode->setNext(NULL);
        _bList[part][cell->getGain()] = nextNode;
    } else if (!isHead && isTail) {  // if the cell is the tail of the list
        Node* prevNode = curNode->getPrev();
        prevNode->setNext(NULL);
        curNode->setPrev(NULL);
    } else {  // if the cell is in the middle of the list
        Node* prevNode = curNode->getPrev();
        Node* nextNode = curNode->getNext();
        prevNode->setNext(nextNode);
        nextNode->setPrev(prevNode);
        curNode->setPrev(NULL);
        curNode->setNext(NULL);
    }
}

void Partitioner::findBest() {
    for (int i = _bestMoveNum; i < _moveNum; ++i) {  // move back
        Cell* backCell = _cellArray[_moveStack[i]];
        ++_partSize[!backCell->getPart()];
        --_partSize[backCell->getPart()];
        for (auto netId : backCell->getNetList()) {
            Net* net = _netArray[netId];
            net->incPartCount(!backCell->getPart());
            net->decPartCount(backCell->getPart());
        }
        backCell->move();
    }
    _cutSize = _initCutSize - _maxAccGain;
    // cout << "_bestMoveNum = " << _bestMoveNum << endl;
    // cout << "_moveNum = " << _moveNum << endl;
    // cout << "_cutSize = " << _cutSize << endl;
}

void Partitioner::reportBList() const {
    cout << "==================== BList " << _moveNum << " ====================" << endl;
    for (int i = 0; i < 2; ++i) {
        cout << "Partition " << i << endl;
        for (auto iter = _bList[i].rbegin(); iter != _bList[i].rend(); ++iter) {
            cout << "Gain: " << iter->first << endl;
            Node* curNode = iter->second;
            while (curNode != NULL) {
                cout << _cellArray[curNode->getId()]->getName() << " ";
                curNode = curNode->getNext();
            }
            cout << endl;
        }
    }
    cout << "=================================================" << endl;
    return;
}

void Partitioner::printSummary() const {
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const {
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
        }
        cout << endl;
    }
    for (int i = 0; i < _netNum; ++i)
        cout << "Net " << _netArray[i]->getName() << " part count: " << _netArray[i]->getPartCount(0) << " " << _netArray[i]->getPartCount(1) << endl;
    return;
}

void Partitioner::reportCell() const {
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[netList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream& outFile) {
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear() {
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}
