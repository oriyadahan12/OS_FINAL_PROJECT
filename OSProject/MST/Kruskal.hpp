#include "MST_Strategy.hpp"
#include "../DataStruct/UnionFind.hpp"

class Kruskal : public MST_Strategy
{
public:
    Graph* operator()(Graph *g);
};
