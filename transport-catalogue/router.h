#pragma once

#include "graph.h"
#include "transport_router.pb.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graph {

template <typename Weight>
class Router {
private:
    using Graph = DirectedWeightedGraph<Weight>;

public:
    struct RouteInternalData {
        Weight weight;
        std::optional<EdgeId> prev_edge;
    };
    using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

    explicit Router(const Graph& graph);    

    Router(const transport_router_proto::RouterDataBase& db, const Graph& graph);

    struct RouteInfo {
        Weight weight;
        std::vector<EdgeId> edges;
    };

    std::optional<RouteInfo>BuildRoute(VertexId from, VertexId to) const;

    const std::vector<std::vector<std::optional<RouteInternalData>>>& GetRoutesInternalData() const;

private:
    void InitializeRoutesInternalData(const Graph& graph) {
        const size_t vertex_count = graph.GetVertexCount();
        for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
            routes_internal_data_[vertex][vertex] = RouteInternalData{ZERO_WEIGHT, std::nullopt};
            for (const EdgeId edge_id : graph.GetIncidentEdges(vertex)) {
                const auto& edge = graph.GetEdge(edge_id);
                if (edge.weight < ZERO_WEIGHT) {
                    throw std::domain_error("Edges' weights should be non-negative");
                }
                auto& route_internal_data = routes_internal_data_[vertex][edge.to];
                if (!route_internal_data || route_internal_data->weight > edge.weight) {
                    route_internal_data = RouteInternalData{edge.weight, edge_id};
                }
            }
        }
    }

    void RelaxRoute(VertexId vertex_from, VertexId vertex_to, const RouteInternalData& route_from,
                    const RouteInternalData& route_to) {
        auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];
        const Weight candidate_weight = route_from.weight + route_to.weight;
        if (!route_relaxing || candidate_weight < route_relaxing->weight) {
            route_relaxing = {candidate_weight,
                              route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge};
        }
    }

    void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
        for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
            if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) {
                for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                    if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) {
                        RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                    }
                }
            }
        }
    }

    static constexpr Weight ZERO_WEIGHT{};
    const Graph& graph_;
    RoutesInternalData routes_internal_data_;
};

template <typename Weight>
Router<Weight>::Router(const Graph& graph)
    : graph_(graph)
    , routes_internal_data_(graph.GetVertexCount(),
                            std::vector<std::optional<RouteInternalData>>(graph.GetVertexCount()))
{
    InitializeRoutesInternalData(graph);

    const size_t vertex_count = graph.GetVertexCount();
    for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
        RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
    }
}

template <typename Weight>
Router<Weight>::Router(const transport_router_proto::RouterDataBase& db, const Graph& graph)
    :graph_(graph)
{
    std::vector<std::optional<RouteInternalData>> temp_data_vector;
    for (auto& data_vector : db.routes_internal_data())
    {
        for (auto& data : data_vector.data())
        {
            RouteInternalData temp_data;
            if (data.optional_data_case() == 1)
            {
                temp_data_vector.push_back({});
            }
            else
            {
                temp_data.weight.bus_name = data.data().rid_weight().bus_name();
                temp_data.weight.span_count = data.data().rid_weight().span_count();
                temp_data.weight.total_time = data.data().rid_weight().total_time();
                if (data.data().prev_edge_case() == 3)
                {
                    temp_data.prev_edge = data.data().edgeid();
                }
                temp_data_vector.push_back(temp_data);
            }
        }
        routes_internal_data_.push_back(std::move(temp_data_vector));
    }
}

template <typename Weight>
std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from,
                                                                             VertexId to) const {
    const auto& route_internal_data = routes_internal_data_.at(from).at(to);
    if (!route_internal_data) {
        return std::nullopt;
    }
    const Weight weight = route_internal_data->weight;
    std::vector<EdgeId> edges;
    for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
         edge_id;
         edge_id = routes_internal_data_[from][graph_.GetEdge(*edge_id).from]->prev_edge)
    {
        edges.push_back(*edge_id);
    }
    std::reverse(edges.begin(), edges.end());

    return RouteInfo{weight, std::move(edges)};
}

template <typename Weight>const typename Router<Weight>::RoutesInternalData&
Router<Weight>::GetRoutesInternalData() const 
{
    return routes_internal_data_;
}
}  // namespace graph