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
		JsonReader(transport_catalogue::Transport_catalogue& tc,
			request_h::RequestHandler& rq,
			renderer::MapRenderer& renderer,
			transport_router::TransportRouter& troute,
			std::istream& input = std::cin);

		void BeginToMakeBase();

		void WriteStopsToBase(const json::Array& arr);

		void WriteBusesToBase(const json::Array& arr);

		void ResponsesToRequests(std::ostream& out = std::cout);
		
		json::Node Requests(const json::Dict& dict, const request_h::RequestHandler& rh);

		void PushRenderSettings(const json::Dict& settings);

		void PushRouteSettings(const json::Dict& settings);

	private:
		transport_catalogue::Transport_catalogue& tc_;
		request_h::RequestHandler& rq_;
		renderer::MapRenderer& renderer_;
		transport_router::TransportRouter& troute_;
		json::Array base_reqs_, stat_reqs_;
		json::Dict render_info_;
		json::Dict route_settings_;
	};
}