#include "request_handler.h"

using namespace request_h;
using namespace transport_catalogue;
using namespace renderer;
using namespace transport_router;

RequestHandler::RequestHandler(const serialization::Serialization& serializator, 
						       const Transport_catalogue& tc, 
							   const MapRenderer& renderer, 
							   const TransportRouter& troute)
	: serializator_(serializator),
	tc_(tc),
	renderer_(renderer),
	troute_(troute)
{
}

std::optional<domain::StopsForBusResponse> RequestHandler::GetBusStat(const std::string_view& bus_name) const
{
	auto stat = tc_.GetBusInfo(std::string(bus_name));
	if (stat.stopsnumber == 0)
	{
		return {};
	}
	else
	{
		return stat;
	}
}

const std::unordered_set <std::string_view> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
	auto buses = tc_.GetStopsForBuses(std::string(stop_name));
	if (buses.empty())
	{
		return {};
	}
	else
	{
		return buses;
	}
}

svg::Document RequestHandler::RenderMap() const
{
	svg::Document doc;
	auto const buses = tc_.GetBuses();
		renderer_.GetPictures(buses).Draw(doc);
	return doc;
}

void RequestHandler::RenderMap(std::ostream& out)
{
	svg::Document doc;
	auto const buses = tc_.GetBuses();
	renderer_.GetPictures(buses).Draw(doc);
	doc.Render(out);
}

std::optional<domain::ReportRouter>RequestHandler::BuildRoute(const std::string& from, const std::string& to) const
{
	domain::ReportRouter report;

	// если начальная и конечная остановка одинаковые - возвращаем пустой результат
	if (from == to)
	{
		return report;
	}

	domain::Info info; 

	auto edges = troute_.TransportRouter::BuildRoute(from, to);
	if (!edges.has_value())
	{
		return {};
	}

	int wait_time = troute_.GetRouteSettings().wait_time;
	
	for (const auto& edge : edges.value())
	{
		report.total_minutes += edge.total_time;
		info.wait.stop_name = edge.stop_from;
		info.wait.minutes = wait_time;
		info.bus.name = edge.bus_name;
		info.bus.span_count = edge.span_count;
		info.bus.minutes = edge.total_time;
		report.information.push_back(info);
	}
	return report;
}