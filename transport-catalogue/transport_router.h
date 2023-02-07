
// Здесь выбран способ построения по одной вершине на остановку. Соответственно в каждую из них закладывается и время ожидания
// Делаем ребро туда и обратно если маршрут линейный, по каждой паре остановок. Если круговой, то 1->2->3->1

#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include <optional>
#include <map>
#include <memory>

namespace transport_router
{
	constexpr static double KMH_TO_MMIN = 1000.0 / 60.0;

	struct RouteWeight
	{
		std::string_view bus_name;
		double total_time = 0;
		size_t span_count = 0;
	};

	class TransportRouter
	{
	public:
		
		struct RouteSettings // входящие параметры настройки из запросов
		{
			int wait_time = 0;      // в минутах
			double velocity = 0.0;    // в метрах в минуту
		};

		struct RouterEdge // выходная структура данных для маршрута
		{
			std::string_view bus_name;
			std::string_view stop_from;
			std::string_view stop_to;
			double total_time = 0;
			size_t span_count = 0;
		};

		TransportRouter(const transport_catalogue::Transport_catalogue& tc);

		void SetRouteSettings(const RouteSettings& settings_);

		RouteSettings GetRouteSettings() const;

		std::optional<std::vector<RouterEdge>>BuildRoute(const std::string& from, const std::string& to) const;		

		size_t CountVertexNumbers();

		void AddEdges();

		void InitRouter();

	private:
		const transport_catalogue::Transport_catalogue& tc_;
		RouteSettings settings_;

		std::unordered_map<size_t, const domain::Stop*> stopname_by_id_; // нумеруем остановки для графа
		std::unordered_map<std::string_view, size_t> id_by_stopname_; // связываем название остановки с ее номером для поиска по маршруту	

		graph::DirectedWeightedGraph<RouteWeight> graph_;
		std::unique_ptr<graph::Router<RouteWeight>> router_;

		graph::Edge<RouteWeight>MakeEdge(std::string_view route_name, const domain::Bus* route, size_t stop_from_index, size_t stop_to_index);

		double ComputeRouteTime(const domain::Bus* route, size_t stop_from_index, size_t stop_to_index) const;
	};


	bool operator<(const RouteWeight& left, const RouteWeight& right);
	bool operator>(const RouteWeight& left, const RouteWeight& right);
	RouteWeight operator+(const RouteWeight& left, const RouteWeight& right);
}