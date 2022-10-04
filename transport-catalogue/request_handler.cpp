#include "request_handler.h"

using namespace request_h;
using namespace transport_catalogue;
using namespace renderer;

RequestHandler::RequestHandler(const Transport_catalogue& tc, const renderer::MapRenderer& renderer)
	: tc_(tc),
	renderer_(renderer)
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