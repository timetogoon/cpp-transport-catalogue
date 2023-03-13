#pragma once

#include <filesystem>
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "svg.pb.h"
#include "graph.pb.h"

namespace serialization
{
	class Serialization 
	{
	public:
        using Path = std::filesystem::path;

        Serialization(transport_catalogue::Transport_catalogue& transport_catalogue,
                      renderer::MapRenderer& map_renderer,
                      transport_router::TransportRouter& transport_router);

        void SetSetting(const Path& path_to_base);
        void CreateBase();
        void AccessBase();

    private:
        Path path_to_base_;
        transport_catalogue::Transport_catalogue& transport_catalogue_;
        renderer::MapRenderer& map_renderer_;
        transport_router::TransportRouter& transport_router_;
        mutable transport_catalogue_proto::TransportCatalogue base_;

        transport_catalogue_proto::Stop SaveStop(const domain::Stop& stop) const;
        transport_catalogue_proto::Distance SaveDistance(domain::Stop* from, domain::Stop* to, uint64_t distance) const;
        transport_catalogue_proto::Bus SaveBus(const domain::Bus* bus) const;
        void SaveStops();
        void SaveDistance();
        void SaveBuses();

        void LoadStop(const transport_catalogue_proto::Stop& stop);
        void LoadDistance(const transport_catalogue_proto::Distance& from_to_distance);
        void LoadBus(const transport_catalogue_proto::Bus& bus);
        void LoadStops();
        void LoadDistances();
        void LoadBuses();

        void SaveRenderSettings();
        void LoadRenderSettings();

        void SaveRouteSettings();
        void SaveIdAndStopnames();
        void SaveGraph();
        void SaveRouter();

        void LoadRouterSettings();
        void LoadRouterIdsAndStops();
        void LoadGraph();
        void LoadRouter();

        static svg_serialize::Point MakeProtoPoint(const svg::Point& point);
        static svg::Point MakePoint(const svg_serialize::Point& p_point);

        static svg_serialize::Color MakeProtoColor(const svg::Color& color);
        static svg::Color MakeColor(const svg_serialize::Color& p_color);
	};
}