#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>
using namespace std;

class Terminal {
   public:
    // constructor and destructor
    Terminal(string& name, int x, int y)
        : _name(name), _x(x), _y(y) {}
    virtual ~Terminal() {}

    // basic access methods
    const string getName() const { return _name; }
    virtual const int getX() const { return _x; }
    virtual const int getY() const { return _y; }

    // set functions
    void setName(string& name) { _name = name; }

   protected:
    string _name;  // module name
    int _x;        // x coordinate of the module
    int _y;        // y coordinate of the module
};

class Node {
    friend class Block;

   public:
    // Constructor and destructor
    Node(const int& id)
        : _id(id), _parent(NULL), _rchild(NULL), _lchild(NULL), _x(0), _y(0) {}
    ~Node() {}

    // Basic access methods
    const int getId() const { return _id; }
    Node* getParent() const { return _parent; }
    Node* getRChild() const { return _rchild; }
    Node* getLChild() const { return _lchild; }
    const int getX() const { return _x; }
    const int getY() const { return _y; }
    const int getArea() const { return _area; }
    const bool getSide() const { return _side; }

    // Set functions
    void setId(const int id) { _id = id; }
    void setParent(Node* parent) { _parent = parent; }
    void setRChild(Node* rchild) { _rchild = rchild; }
    void setLChild(Node* lchild) { _lchild = lchild; }
    void setX(const int x) { _x = x; }
    void setY(const int y) { _y = y; }
    void setArea(const int area) { _area = area; }
    void setSide(const bool side) { _side = side; }

   private:
    int _id;
    Node* _parent;  // pointer to the parent node
    Node* _rchild;  // pointer to the right child
    Node* _lchild;  // pointer to the left child
    int _x;         // x coordinate of the node
    int _y;         // y coordinate of the node
    int _area;      // area of the node
    bool _side;     // 0: left, 1: right
};

class Block : public Terminal  // inherit from Terminal
{
   public:
    // constructor and destructor
    Block(string& name, int w, int h)
        : Terminal(name, 0, 0), _w(w), _h(h), _rotate(false), _node(NULL) {}
    ~Block() {}

    // basic access methods
    const int getWidth() { return _rotate ? _h : _w; }
    const int getHeight() { return _rotate ? _w : _h; }
    const int getArea() const { return _h * _w; }
    const int getX() const override { return _node->getX() + _w / 2; }
    const int getY() const override { return _node->getY() + _h / 2; }
    const bool getRotate() const { return _rotate; }
    Node* getNode() { return _node; }

    // set functions
    void setWidth(int w) { _w = w; }
    void setHeight(int h) { _h = h; }
    void setNode(Node* node) { _node = node; }
    void rotate() { _rotate = !_rotate; }

   private:
    int _w;       // width of the block
    int _h;       // height of the block
    bool _rotate; // whether the block is rotated
    Node* _node;  // pointer to the node in the tree
};

class Net {
   public:
    // constructor and destructor
    Net() {}
    ~Net() {}

    // basic access methods
    const int getDegree() { return _degree; }
    const int getHPWL() { return _HPWL; }
    Terminal* getTerm(int i) { return _termList[i]; }
    const vector<Terminal*> getTermList() { return _termList; }

    // modify methods
    void addTerm(Terminal* term) { _termList.push_back(term); }
    void setDegree(int degree) { _degree = degree; }

    // other member functions
    void calcHPWL();

   private:
    int     _degree;                    // degree of the net
    double  _HPWL;                      // half-perimeter wire length
    vector<Terminal*> _termList;        // list of terminals the net is connected to
};

#endif  // MODULE_H
