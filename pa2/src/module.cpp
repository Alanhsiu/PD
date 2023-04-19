#include <vector>
#include <string>
#include <climits>
#include "module.h"

using namespace std;

void Net::calcHPWL(){
    int minX = INT_MAX;
    int maxX = 0;
    int minY = INT_MAX;
    int maxY = 0;
    for(int i = 0; i < _degree; ++i){
        if(_termList[i]->getX() < minX)
            minX = _termList[i]->getX();
        if(_termList[i]->getX() > maxX)
            maxX = _termList[i]->getX();
        if(_termList[i]->getY() < minY)
            minY = _termList[i]->getY();
        if(_termList[i]->getY() > maxY)
            maxY = _termList[i]->getY();
    }
    this->_HPWL = (maxX - minX) + (maxY - minY);
}