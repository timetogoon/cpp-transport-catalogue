#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"
#include "transport_router.h"

namespace json_reader 
{
	class JsonReader final
	{
	public:
		JsonReader() = delete;
		JsonReader(serialization::Serialization& serializator,
			transport_catalogue::Transport_catalogue& tc,
			request_h::RequestHandler& rq,
			renderer::MapRenderer& renderer,
			transport_router::TransportRouter& troute);

		void BeginToMakeBase(std::istream& input = std::cin);

		void WriteStopsToBase(const json::Array& arr);

		void WriteBusesToBase(const json::Array& arr);

		void ResponsesToRequests(std::ostream& out = std::cout);
		
		json::Node Requests(const json::Dict& dict, const request_h::RequestHandler& rh);

		void PushRenderSettings(const json::Dict& settings);

		void PushRouteSettings(const json::Dict& settings);

		void PushSerializationSettings(const json::Dict& serialization_settings_);

		void ReadRequests(std::istream& in);

	private:
		serialization::Serialization& serializator_;
		transport_catalogue::Transport_catalogue& tc_;
		request_h::RequestHandler& rq_;
		renderer::MapRenderer& renderer_;
		transport_router::TransportRouter& troute_;
		json::Array base_reqs_, stat_reqs_;
		std::optional<json::Dict>render_info_;
		std::optional<json::Dict>route_settings_;
		std::optional<json::Dict>serialization_settings_;
	};
}