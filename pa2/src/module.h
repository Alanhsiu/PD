#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <string>
using namespace std;

class Terminal
{
public:
    // constructor and destructor
    Terminal(string& name, int x, int y) :
        _name(name), _x1(x), _y1(y), _x2(x), _y2(y) { }
    ~Terminal()  { }

    // basic access methods
    const string getName()  { return _name; }
    const int getX1()    { return _x1; }
    const int getX2()    { return _x2; }
    const int getY1()    { return _y1; }
    const int getY2()    { return _y2; }

    // set functions
    void setName(string& name) { _name = name; }
    void setPos(int x1, int y1, int x2, int y2) {
        _x1 = x1;   _y1 = y1;
        _x2 = x2;   _y2 = y2;
    }

protected:
    string      _name;      // module name
    int      _x1;        // min x coordinate of the terminal
    int      _y1;        // min y coordinate of the terminal
    int      _x2;        // max x coordinate of the terminal
    int      _y2;        // max y coordinate of the terminal
};


class Block : public Terminal
{
public:
    // constructor and destructor
    Block(string& name, int w, int h) :
        Terminal(name, 0, 0), _w(w), _h(h) { }
    ~Block() { }

    // basic access methods
    const int getWidth(bool rotate = false)  { return rotate? _h: _w; }
    const int getHeight(bool rotate = false) { return rotate? _w: _h; }
    const int getArea()  { return _h * _w; }
    static int getMaxX() { return _maxX; }
    static int getMaxY() { return _maxY; }

    // set functions
    void setWidth(int w)         { _w = w; }
    void setHeight(int h)        { _h = h; }
    static void setMaxX(int x)   { _maxX = x; }
    static void setMaxY(int y)   { _maxY = y; }


private:
    int          _w;         // width of the block
    int          _h;         // height of the block
    static int   _maxX;      // maximum x coordinate for all blocks
    static int   _maxY;      // maximum y coordinate for all blocks
};


class Net
{
public:
    // constructor and destructor
    Net()   { }
    ~Net()  { }

    // basic access methods
    const vector<Terminal*> getTermList()   { return _termList; }

    // modify methods
    void addTerm(Terminal* term) { _termList.push_back(term); }

    // other member functions
    double calcHPWL();

private:
    vector<Terminal*>   _termList;  // list of terminals the net is connected to
};

#endif  // MODULE_H
