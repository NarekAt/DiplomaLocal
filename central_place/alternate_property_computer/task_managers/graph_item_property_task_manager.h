/**
 * @file task_managers/graph_item_property_task_manager.h
 * @brief declaration of graph_item_property_task_manager class.
 */

#pragma once

#include "types.h"
#include "graph_types.h"
#include "graph.h"
#include "igraph_task_manager.h"

#include "utility.h"

class graph_item_property_task_manager : public igraph_task_manager
{
public:
    graph_item_property_task_manager(const graph_types::graph& g,
                        const apt_list& apts,
                        bool calcDistr,
                        std::ofstream& logger);

    void run();

    ~graph_item_property_task_manager();

private:
    apt_list m_graph_item_related_apts;
    bool m_calc_distr;
};

