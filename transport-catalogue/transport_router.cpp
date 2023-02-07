#include "transport_router.h"

using namespace std;

namespace transport_router
{
	TransportRouter::TransportRouter(const transport_catalogue::Transport_catalogue& tc)
		: tc_(tc)
	{		
	}
	
	void TransportRouter::SetRouteSettings(const RouteSettings& settings)
	{
		settings_ = settings;
	}

	TransportRouter::RouteSettings TransportRouter::GetRouteSettings() const
	{
		return settings_;
	}

	size_t TransportRouter::CountVertexNumbers() // подсчет кол-ва вершин для постройки графа
													   // и их нумерация 
	{
		size_t vertex_count = 0;
		const auto& stops = tc_.GetStopsPtr();
		stopname_by_id_.reserve(stops.size());
		id_by_stopname_.reserve(stops.size());
		for (const auto& stop : stops)
		{
			id_by_stopname_.insert({stop.first, vertex_count});
			stopname_by_id_.insert({vertex_count++, stop.second});
		}
		return vertex_count;
	}

	void TransportRouter::AddEdges() // добавляем ребра в граф
	{
		for (const auto& [route_name, route] : tc_.GetBusesPtr()) // список указателей остановок
		{
			size_t stops_count = route->buses_stops.size();
			for (size_t i = 0; i < stops_count; ++i) // цикл по остановкам
			{
				double route_time = settings_.wait_time; 
				double route_time_back = settings_.wait_time;
				for (size_t j = i + 1; j < stops_count; ++j)  // цикл по остановкам + 1
				{
					graph::Edge<RouteWeight>edge = MakeEdge(route_name, route, i, j);
					route_time += ComputeRouteTime(route, j - 1, j);  // считаем время
					edge.weight.total_time = route_time;
					graph_.AddEdge(edge);

					if (!route->ring) // если маршрут линейный
					{
						edge = MakeEdge(route_name, route, j, i);
						route_time_back += ComputeRouteTime(route, j, j - 1);
						edge.weight.total_time = route_time_back;
						graph_.AddEdge(edge);
					}
				}
			}
		}
	}	

	std::optional<std::vector<TransportRouter::RouterEdge>>TransportRouter::BuildRoute(
															   const std::string& from, 
															   const std::string& to) const
	{																// самая главная функция тут!
		{
			std::vector<RouterEdge> result;
			const auto from_id = id_by_stopname_.at(from);
			const auto to_id = id_by_stopname_.at(to);
			const auto& route = router_->BuildRoute(from_id, to_id);

			if (!route) // если результат nullopt, то возвращаем пустое множество
			{
				return {};
			}

			for (auto edge_id : route->edges) // исследуем все полученные ребра маршрута и пакуем
			{
				const auto& edge = graph_.GetEdge(edge_id);
				RouterEdge route_edge;
				route_edge.bus_name = edge.weight.bus_name;
				route_edge.stop_from = stopname_by_id_.at(edge.from)->name;
				route_edge.stop_to = stopname_by_id_.at(edge.to)->name;
				route_edge.span_count = edge.weight.span_count;
				route_edge.total_time = edge.weight.total_time;
				result.push_back(route_edge);
			}
			return result;
		}
	}

	void TransportRouter::InitRouter() //инициализация графа и маршрута
	{
		graph_ = graph::DirectedWeightedGraph<RouteWeight>(CountVertexNumbers());
		AddEdges();
		router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
	}

	graph::Edge<RouteWeight>TransportRouter::MakeEdge(string_view route_name, const domain::Bus* route,
													  size_t stop_from_index, size_t stop_to_index) 
	{													// записываем инфу о ребре (имя маршрута, кол-во пересадок),
														// а также расставляем верный id из контейнера по id из маршрута
		graph::Edge<RouteWeight>edge;
		edge.from = id_by_stopname_.at(route->buses_stops[stop_from_index]->name);
		edge.to = id_by_stopname_.at(route->buses_stops[stop_to_index]->name);
		edge.weight.bus_name = route_name;
		edge.weight.span_count = stop_to_index - stop_from_index;
		return edge;
	}

	double TransportRouter::ComputeRouteTime(const domain::Bus* route, size_t stop_from_index,
											 size_t stop_to_index) const 
	{													// подсчет времени маршрута
		auto split_distance =
			tc_.GetDistance(route->buses_stops.at(stop_from_index)->name,
				route->buses_stops.at(stop_to_index)->name);
		return split_distance / settings_.velocity;
	}

	//-------------Операторы для работы в графе с типом RouteWeight, а то ругается---------------
	bool operator<(const RouteWeight& left, const RouteWeight& right)
	{
		return left.total_time < right.total_time;
	}

	bool operator>(const RouteWeight& left, const RouteWeight& right) 
	{
		return left.total_time > right.total_time;
	}

	RouteWeight operator+(const RouteWeight& left, const RouteWeight& right)
	{
		RouteWeight result;
		result.total_time = left.total_time + right.total_time;
		return result;
	}
}