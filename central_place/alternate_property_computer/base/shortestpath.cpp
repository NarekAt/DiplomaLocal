#include "shortestpath.h"
#include "graph_constants.h"

namespace Base
{

void
ShortestPath::computeImpl()
{
    using namespace graph_types;

    ShortestPathBase::VertexCollection terminalVertices;

    for (vertex v = constants::VERTEX_0; v < graph_.size(); ++v)
    {
        terminalVertices.push_back(v);
    }

    ShortestPathBase::PathLengthProp lengths;
    ShortestPathBase::calculate(graph_, terminalVertices, lengths);

    // TODO: get rid of this !!
    // Enhance ShortestPathBase interface
    for (unsigned i = 0; i != lengths.size(); ++i)
    {
        results_[i] = static_cast<unsigned>(lengths[i]);
    }
}

alternate_property_type
ShortestPath::typeImpl() const
{
    return SHORTEST_PATH;
}

ShortestPath::ResultType
ShortestPath::getDistributionImpl() const
{
    ResultType distributions;

    for (const auto& r : results_)
    {
        if (r >= distributions.size())
            distributions.resize(r + 1);

        ++(distributions[r]);
    }
    return distributions;
}


} // namespace Base
