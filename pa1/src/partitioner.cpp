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
    init();
}

void Partitioner::init() {
    int netSize = _netArray.size();
    int cellSize = _cellArray.size();
    int halfCellSize = cellSize / 2;
    _maxPinNum = 0;

    // init cell part and net info
    for (int i = 0; i < cellSize; ++i) {
        Cell* cell = _cellArray[i];
        if (i < halfCellSize) {  // put half of the cells in part A
            cell->setPart(0);
            cell->setGain(0);
            // update net info
            vector<int> netList = cell->getNetList();
            for (auto id : netList) {
                _netArray[id]->incPartCount(0);
            }
        } else {  // put the other half of the cells in part B
            cell->setPart(1);
            cell->setGain(0);
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
    _partSize[0] = halfCellSize;
    _partSize[1] = cellSize - halfCellSize;
    // init cutsize
    for (int i = 0; i < netSize; ++i)
        if (_netArray[i]->getPartCount(0) && _netArray[i]->getPartCount(1))
            ++_cutSize;

    // init gain, maxGain, and maxGainList
    for (int i = 0; i < cellSize; ++i) {
        Cell* cell = _cellArray[i];
        vector<int> netList = cell->getNetList();
        int netListSize = netList.size();
        for (int j = 0; j < netListSize; ++j) {
            int netId = netList[j];
            Net* net = _netArray[netId];
            int F = 0, T = 0;
            if (cell->getPart())
                F = net->getPartCount(0), T = net->getPartCount(1);
            else
                F = net->getPartCount(1), T = net->getPartCount(0);
            if (F == 1)
                cell->incGain();
            if (T == 0)
                cell->decGain();
        }
    }
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
