#include "Kruskal.hpp"


    Graph* Kruskal::operator()(Graph *g){ 
        Graph* mst = new Graph(*g, false); // Create a new graph with the same vertices as the input graph but no edges

        std::vector<Edge> edges;  // Create a vector to store the edges
        for (auto e = g->edgesBegin(); e != g->edgesEnd(); e++)
        {
            edges.push_back(*e);
        }
        std::sort(edges.begin(), edges.end());  // Sort the edges in non decreasing order of weight

        UnionFind uf(g->numVertices());
        for (auto e : edges)
        {
            if (uf.find(e.getStart().getId()) != uf.find(e.getEnd().getId())) //for each edge E = u,v in G taken in non decreasing order of weight, if u and v are not in the same set, add E to the MST
            {
                mst->addEdge(e);
                uf.Union(e.getStart().getId(), e.getEnd().getId());
            }
        }
        std::vector<std::vector<size_t>> dist, per;
        std::tie(dist, per) = mst->floydWarshall(); // Get the distance and parent matrices of the MST
        //update distance and parent matrices in mst
        mst->setDistances(dist);
        mst->setParent(per);
        return mst;
    }

