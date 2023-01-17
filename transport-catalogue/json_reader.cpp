#include "json_reader.h"


using namespace json_reader;
using namespace std::string_literals;

JsonReader::JsonReader(transport_catalogue::Transport_catalogue& tc,
					   request_h::RequestHandler& rq,
					   renderer::MapRenderer& renderer,
					   std::istream& input)
		 : tc_(tc),
		   rq_(rq),
	       renderer_(renderer)
{
	auto allreq = json::Load(input).GetRoot().AsMap();
	base_reqs_ = allreq.at("base_requests").AsArray();
	render_info_ = allreq.at("render_settings").AsMap();
	stat_reqs_ = allreq.at("stat_requests").AsArray();
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

void JsonReader::BeginToMakeBase() //распределние записей в базу
{
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
	JsonReader::WriteStopsToBase(onlystops); //остановки
	JsonReader::WriteBusesToBase(onlybuses); //маршруты
	JsonReader::PushRenderSettings(render_info_); //настройки рендеринга
}

json::Node JsonReader::Requests(const json::Dict& dict, const request_h::RequestHandler& rh) //обработка запросов к базе
{	
	json::Array temparr;
	json::Builder builder;
	if (dict.at("type"s) == "Stop"s)
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
	else if (dict.at("type"s) == "Bus"s)
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
	else if (dict.at("type"s) == "Map"s)
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
	else
	{
		throw std::logic_error("Invalid request"s);
	};
}

void JsonReader::ResponsesToRequests(std::ostream& out) //ответ на запросы к базе
{
	json::Array answer;
	for (const auto& inquiries : stat_reqs_) 
	{
		answer.push_back(Requests(inquiries.AsMap(), rq_));
	}
	const json::Document report(answer);
	json::Print(report, out);
}