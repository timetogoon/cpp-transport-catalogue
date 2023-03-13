#include "json_reader.h"


using namespace json_reader;
using namespace std::string_literals;

JsonReader::JsonReader(serialization::Serialization& serializator,
					   transport_catalogue::Transport_catalogue& tc,
					   request_h::RequestHandler& rq,
					   renderer::MapRenderer& renderer,
		               transport_router::TransportRouter& troute)
		 : serializator_(serializator),
		   tc_(tc),
		   rq_(rq),
	       renderer_(renderer),
		   troute_(troute)
{	
}

void JsonReader::WriteStopsToBase(const json::Array& arr) //запись остановок в базу
{
	for (auto& node : arr) 
	{
		auto& map = node.AsMap();
			tc_.AddStop(map.at("name"s).AsString(),
				map.at("latitude"s).AsDouble(),
				map.at("longitude"s).AsDouble());
	}
	
	for (auto& node : arr) 
	{
		auto& map = node.AsMap();
		if (map.count("road_distances"s))
		{
			for (auto& [name, distance] : map.at("road_distances"s).AsMap())
				tc_.SetDistances(map.at("name"s).AsString(), name, distance.AsInt());
		}
	}
	
	auto distances = tc_.GetDistancesAll();
	for (auto& distance : distances)
	{
		const auto& name_from = distance.first.first->name;
		const auto& name_to = distance.first.second->name;

		auto tmp = tc_.GetDistance(name_to, name_from);

		if (tmp == 0)
		{
			tc_.SetDistances(name_to, name_from, distance.second);
		}
	}	
}

void JsonReader::WriteBusesToBase(const json::Array& arr) //запись маршрутов в базу
{
	std::vector<const Stop*> bus_stops;
	for (auto& node : arr) 
	{
		auto& map = node.AsMap();
		for (auto& stop : map.at("stops"s).AsArray()) 
		{
			bus_stops.push_back(tc_.GetStopptr(stop.AsString()));
		}
		tc_.AddBus(map.at("name"s).AsString(), 
			bus_stops,
			map.at("is_roundtrip"s).AsBool());
		bus_stops.clear();
	}
}

uint8_t ReadByte(const json::Node& json) {
	int int_value = json.AsInt();
	uint8_t byte = static_cast<uint8_t>(int_value);
	if (static_cast<int>(byte) == int_value) {
		return byte;
	}
	throw std::out_of_range(std::to_string(int_value) + " is out of byte range"s);
}

svg::Color ReadColor(const json::Node& json) {
	if (json.IsArray()) {
		const auto& arr = json.AsArray();
		if (arr.size() == 3) {  // Rgb
			return svg::Rgb(ReadByte(arr[0]), ReadByte(arr[1]), ReadByte(arr[2]));
		}
		else if (arr.size() == 4) {  // Rgba
			return svg::Rgba(ReadByte(arr[0]), ReadByte(arr[1]), ReadByte(arr[2]),
				arr[3].AsDouble());
		}
	}
	else if (json.IsString()) {
		return json.AsString();
	}
	else if (json.IsNull()) {
		return svg::NoneColor;
	}	
	return {};
}

svg::Point ReadPoint(const json::Array& json) {
	if (json.size() != 2) {
		throw std::invalid_argument("Point array must have exactly 2 elements");
	}
	return svg::Point{ json[0].AsDouble(), json[1].AsDouble() };
}

std::vector<svg::Color> ReadColors(const json::Array& json) {
	std::vector<svg::Color> colors;
	colors.reserve(json.size());

	for (const auto& item : json) {
		colors.emplace_back(ReadColor(item));
	}

	return colors;
}

void JsonReader::PushRenderSettings(const json::Dict& settings)
{
	renderer::RenderSettings rs;

	rs.width = settings.at("width"s).AsDouble();
	rs.height = settings.at("height"s).AsDouble();
	rs.padding = settings.at("padding"s).AsDouble();

	rs.line_width = settings.at("line_width"s).AsDouble();
	rs.stop_radius = settings.at("stop_radius"s).AsDouble();

	rs.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
	rs.bus_label_offset = ReadPoint(settings.at("bus_label_offset").AsArray());

	rs.stop_label_font_size = settings.at("stop_label_font_size"s).AsInt();
	rs.stop_label_offset = ReadPoint(settings.at("stop_label_offset"s).AsArray());

	rs.underlayer_color = ReadColor(settings.at("underlayer_color"s));
	rs.underlayer_width = settings.at("underlayer_width"s).AsDouble();

	rs.color_palette = ReadColors(settings.at("color_palette"s).AsArray());
	renderer_.SetRenderSettings(rs);
}

void JsonReader::PushRouteSettings(const json::Dict& settings)
{
	transport_router::TransportRouter::RouteSettings routesettings;
	routesettings.wait_time = settings.at("bus_wait_time"s).AsInt();
	routesettings.velocity = settings.at("bus_velocity").AsDouble() * transport_router::KMH_TO_MMIN;
	troute_.SetRouteSettings(routesettings);
}

void JsonReader::PushSerializationSettings(const json::Dict& serialization_settings_)
{	
	serializator_.SetSetting(serialization_settings_.at("file"s).AsString());
}

void JsonReader::BeginToMakeBase(std::istream& input) // распределние записей в базу
{
	auto allreq = json::Load(input).GetRoot().AsMap();
	base_reqs_ = allreq.at("base_requests"s).AsArray();

	if (allreq.contains("render_settings"s))
	{
		render_info_ = allreq.at("render_settings"s).AsMap();
	}	

	if (allreq.contains("routing_settings"s))
	{
		route_settings_ = allreq.at("routing_settings"s).AsMap();
	}
	
	if (allreq.contains("serialization_settings"s))
	{
		serialization_settings_ = allreq.at("serialization_settings"s).AsMap();
	}	

	json::Array onlystops, onlybuses;
	for (auto& req : base_reqs_)
	{
		auto& type = req.AsMap().at("type").AsString();
		if (type == "Stop")
		{
			onlystops.push_back(req);
		}
		else
		{
			onlybuses.push_back(req);
		}
	}
	JsonReader::WriteStopsToBase(onlystops); // остановки
	JsonReader::WriteBusesToBase(onlybuses); // маршруты

	if (render_info_.has_value())
	{
		JsonReader::PushRenderSettings(render_info_.value()); // настройки параметров рендеринга
	}
	
	if (route_settings_.has_value())
	{
		JsonReader::PushRouteSettings(route_settings_.value()); // настройка параметров маршрута
	}

	if (serialization_settings_.has_value())
	{
		JsonReader::PushSerializationSettings(serialization_settings_.value()); // настройка параметров сериализации
	}	
	
	troute_.InitRouter(); // инициализация графа
	serializator_.CreateBase();
}

json::Node JsonReader::Requests(const json::Dict& dict, const request_h::RequestHandler& rh) // обработка запросов к базе
{	
	json::Array temparr;
	json::Builder builder;
	if (dict.at("type"s) == "Stop"s) // запрос - найти информацию об остановке
	{
		if (tc_.FindStop(dict.at("name"s).AsString()).name.empty())
		{			
			builder.StartDict()
				.Key("request_id"s).Value(dict.at("id"s).AsInt())
				.Key("error_message"s).Value("not found"s)
				.EndDict();
			return builder.Build();
		}
		auto result = rh.GetBusesByStop(dict.at("name"s).AsString());
		std::set<std::string_view> temp(result.begin(), result.end());
		if (!result.empty())
		{
			for (auto& r : temp)
			{
				temparr.push_back(std::string(r));
			}
			builder
				.StartDict()
				.Key("buses"s).StartArray();
			for (size_t i = 0; i < temparr.size(); i++)
			{
				builder.Value(temparr[i]);
			}
			builder
				.EndArray()
				.Key("request_id"s).Value(dict.at("id"s).AsInt())
				.EndDict();
			return builder.Build();				
		}
		else
		{
			builder
				.StartDict()
				.Key("request_id"s).Value(dict.at("id"s).AsInt())
				.Key("buses"s).StartArray();
			for (size_t i = 0; i < temparr.size(); i++)
			{
				builder.Value(temparr[i]);
			}
			builder
				.EndArray()
				.EndDict();
			return builder.Build();
		}
	}
	else if (dict.at("type"s) == "Bus"s) // запрос - найти информацию о маршруте
	{
		auto result = rh.GetBusStat(dict.at("name"s).AsString());
		if (result.has_value())
		{
			builder.StartDict()
				.Key("curvature"s).Value(result.value().curvature)
				.Key("request_id"s).Value(dict.at("id"s).AsInt())
				.Key("route_length"s).Value(result.value().routelength)
				.Key("stop_count"s).Value(static_cast<int>(result.value().stopsnumber))
				.Key("unique_stop_count"s).Value(static_cast<int>(result.value().uniquestops))
				.EndDict();
			return builder.Build();
		}
		else
		{
			builder.StartDict()
				.Key("request_id"s).Value(dict.at("id"s).AsInt())
				.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build();
			return builder.Build();
		}
	}
	else if (dict.at("type"s) == "Map"s) // запрос - нарисовать карту
	{
		std::ostringstream out;
		rh.RenderMap()
			.Render(out);		
		builder.StartDict()
			.Key("map"s).Value(out.str())
			.Key("request_id"s).Value(dict.at("id"s).AsInt())
			.EndDict();
			return builder.Build();
	}
	else if (dict.at("type"s) == "Route"s) // запрос - найти маршрут
	{		
		auto result = rh.BuildRoute(dict.at("from"s).AsString(), dict.at("to"s).AsString());		
		if (!result.has_value())
		{
			return json::Builder{}.StartDict()
				.Key("request_id"s).Value(dict.at("id"s).AsInt())
				.Key("error_message"s).Value("not found"s)
				.EndDict().Build().AsMap();
		}

		json::Array items; // складируем всё в массив для дальнейшего вывода
		for (const auto& res : result.value().information) 
		{			
			json::Dict wait_elem = json::Builder{}.StartDict()
				.Key("type"s).Value("Wait"s)
				.Key("stop_name"s).Value(std::string(res.wait.stop_name))
				.Key("time"s).Value(res.wait.minutes)
				.EndDict().Build().AsMap();
			json::Dict ride_elem = json::Builder{}.StartDict()
				.Key("type"s).Value("Bus"s)
				.Key("bus"s).Value(std::string(res.bus.name))
				.Key("span_count"s).Value(static_cast<int>(res.bus.span_count))
				.Key("time"s).Value(res.bus.minutes - res.wait.minutes)
				.EndDict().Build().AsMap();
			items.push_back(wait_elem);
			items.push_back(ride_elem);
		}
		 builder.StartDict()
			.Key("request_id"s).Value(dict.at("id"s).AsInt())
			.Key("total_time"s).Value(result.value().total_minutes)
			.Key("items"s).Value(items)
			.EndDict();
		return builder.Build().AsMap();
	}
	else
	{
		throw std::logic_error("Invalid request"s);
	};
}

void JsonReader::ResponsesToRequests(std::ostream& out) // ответы на запросы к базе
{
	json::Array answer;
	for (const auto& inquiries : stat_reqs_) 
	{
		answer.push_back(Requests(inquiries.AsMap(), rq_));
	}
	const json::Document report(answer);
	json::Print(report, out);
}

void JsonReader::ReadRequests(std::istream& in = std::cin)
{
	const auto load = json::Load(in).GetRoot().AsMap();
	stat_reqs_ = load.at("stat_requests").AsArray();
	serialization_settings_ = load.at("serialization_settings"s).AsMap();
	
	JsonReader::PushSerializationSettings(serialization_settings_.value()); // настройка параметров сериализации
		
	serializator_.AccessBase();
	
	troute_.InitRouter();
}