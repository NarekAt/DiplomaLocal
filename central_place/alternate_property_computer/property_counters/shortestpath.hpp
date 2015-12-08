#ifndef INCLUDED__SHORTESTPATH_HPP
#define INCLUDED__SHORTESTPATH_HPP

#include "bfs.h"

#include <map>
#include <boost/shared_ptr.hpp>


class ShortestPath
{
public: // types
    typedef graph_types::BFS                        BFS;
    typedef BFS::WeightedVertex                     WeightedVertex;
    typedef BFS::WeightedVertices                   WeightedVertexCollection;
    typedef BFS::Vertices                           VertexCollection;
    typedef std::vector<double>                     DistanceResultCollection;
    typedef std::map<graph_types::vertex, double>   PathLengthProp;

public: // functions

    // Given a graph g and a subset of its vertices terminalVertices
    // calculates for every vertex of g the shortest path to the closest
    // terminal vertex and storing the calculated path length values in pathLengths.

    static void calculate(const graph_types::graph& g,
                          const VertexCollection& terminalVertices,
                          PathLengthProp& pathLengths);

    static void calculate(const graph_types::graph& g,
                          const VertexCollection& terminalVertices,
                          DistanceResultCollection& pathLengths);

    static void calculate(const graph_types::graph& g,
                          const WeightedVertexCollection& terminalVertices,
                          DistanceResultCollection& pathLengths);
};

#endif
