#include "MST_Strategy.hpp"
#include "../DataStruct/UnionFind.hpp"
#include "MST_Strategy.hpp"
#include <queue>
#include <vector>
#include "../DataStruct/BinaryHeap.hpp"


class Prim : public MST_Strategy
{
public:
    Graph* operator()(Graph *g);
};


class Kruskal : public MST_Strategy
{
public:
    Graph* operator()(Graph *g);
};
