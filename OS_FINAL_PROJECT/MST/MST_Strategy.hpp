#pragma once
#include "../Graph/graph.hpp"

class MST_Strategy
{
public:
    virtual Graph* operator()(Graph *g) = 0;
    virtual ~MST_Strategy() = default;
};