#include "serialization.h"

namespace serialization
{
    Serialization::Serialization(transport_catalogue::Transport_catalogue& transport_catalogue,
        renderer::MapRenderer& map_renderer,
        transport_router::TransportRouter& transport_router)
        : transport_catalogue_(transport_catalogue)
        , map_renderer_(map_renderer)
        , transport_router_(transport_router) 
    {
    }

    void Serialization::SetSetting(const Path& path_to_base) 
    {
        path_to_base_ = path_to_base;
    }

    void Serialization::CreateBase() 
    {
        std::ofstream out_file(path_to_base_, std::ios::binary);
        SaveStops();
        SaveDistance();
        SaveBuses();
        SaveRenderSettings();
        SaveRouteSettings();
        SaveIdAndStopnames();
        SaveGraph();
        SaveRouter();
        base_.SerializeToOstream(&out_file);
    }  

    transport_catalogue_proto::Stop Serialization::SaveStop(const domain::Stop& stop) const
    {
        transport_catalogue_proto::Stop result;
        result.set_name(stop.name);
        result.mutable_coords()->set_lat(stop.coord.lat);
        result.mutable_coords()->set_lng(stop.coord.lng);
        return result;
    }

    transport_catalogue_proto::Distance Serialization::SaveDistance(domain::Stop* from, domain::Stop* to, uint64_t distance) const
    {
        transport_catalogue_proto::Distance result;
        result.set_stop_name_from(from->name);
        result.set_stop_name_to(to->name);
        result.set_distance(distance);
        return result;
    }

    transport_catalogue_proto::Bus Serialization::SaveBus(const domain::Bus* bus) const
    {
        transport_catalogue_proto::Bus result;
        (bus->ring == 1) ? result.set_is_ring(true) : result.set_is_ring(false);
        result.set_name(bus->name);
        for (const auto& stop : bus->buses_stops) 
        {
            result.add_stop_names(stop->name);
        }
        return result;
    }

    void Serialization::SaveStops() 
    {
        transport_catalogue_proto::Stop add_stop;
        transport_catalogue_proto::Coordinates coords;
        for (const auto& stop : transport_catalogue_.GetStops()) 
        {
            add_stop.set_name(stop.name);
            coords.set_lat(stop.coord.lat);
            coords.set_lng(stop.coord.lng);
            *add_stop.mutable_coords() = coords;
            *base_.add_stops() = add_stop;
        }
    }

    void Serialization::SaveDistance()
    {
        for (const auto& [from_to, distance] : transport_catalogue_.GetDistancesAll())
        {
            *base_.add_stops_distances() = std::move(SaveDistance(from_to.first, from_to.second, distance));
        }
    }

    void Serialization::SaveBuses() 
    {
        for (const auto& bus : transport_catalogue_.GetBusesPtr())
        {
            *base_.add_buses() = std::move(SaveBus(bus.second));
        }
    }

    void Serialization::AccessBase() 
    {
        std::ifstream in_file(path_to_base_, std::ios::binary);
        base_.ParseFromIstream(&in_file);
        LoadStops();
        LoadBuses();
        LoadDistances();
        LoadRenderSettings();
        LoadRouterSettings();
        LoadRouterIdsAndStops();
        LoadGraph();
        LoadRouter();
    }

    void Serialization::LoadStop(const transport_catalogue_proto::Stop& stop) 
    {
        transport_catalogue_.AddStop(stop.name(), stop.coords().lat(), stop.coords().lng());
    }

    void Serialization::LoadDistance(const transport_catalogue_proto::Distance& from_to_distance) 
    {
        const auto& from = transport_catalogue_.FindStop(from_to_distance.stop_name_from());
        const auto& to = transport_catalogue_.FindStop(from_to_distance.stop_name_to());
        transport_catalogue_.SetDistances(from.name, to.name, from_to_distance.distance());
    }

    void Serialization::LoadBus(const transport_catalogue_proto::Bus& bus)
    {
        std::vector<const Stop*> stops;
        for (const auto& stop : bus.stop_names()) 
        {
            stops.emplace_back(transport_catalogue_.GetStopptr(stop));
        }        
        transport_catalogue_.AddBus(bus.name(), stops, bus.is_ring());
    }

    void Serialization::LoadStops() 
    {
        for (int i = 0; i < base_.stops_size(); ++i) 
        {
            LoadStop(base_.stops(i));
        }
    }

    void Serialization::LoadDistances() {
        for (int i = 0; i < base_.stops_distances_size(); ++i)
        {
            LoadDistance(base_.stops_distances(i));
        }
    }

    void Serialization::LoadBuses() {
        for (int i = 0; i < base_.buses_size(); ++i) 
        {
            LoadBus(base_.buses(i));
        }
    }

    //----------------------Сейв и загрузка настроек рендеринга-----------------
    void Serialization::SaveRenderSettings()
    {
        auto p_settings = base_.mutable_render_settings();

        auto settings = map_renderer_.GetRenderSettings();

        svg_serialize::Point p;
        p.set_x(settings.width);
        p.set_y(settings.height);

        *p_settings->mutable_size() = p;

        p_settings->set_padding(settings.padding);

        p_settings->set_line_width(settings.line_width);
        p_settings->set_stop_radius(settings.stop_radius);

        p_settings->set_bus_label_font_size(settings.bus_label_font_size);
        *p_settings->mutable_bus_label_offset() = MakeProtoPoint(settings.bus_label_offset);

        p_settings->set_stop_label_font_size(settings.stop_label_font_size);
        *p_settings->mutable_stop_label_offset() = MakeProtoPoint(settings.stop_label_offset);

        *p_settings->mutable_underlayer_color() = MakeProtoColor(settings.underlayer_color);
        p_settings->set_underlayer_width(settings.underlayer_width);

        for (auto& color : settings.color_palette) {
            *p_settings->add_color_palette() = MakeProtoColor(color);
        }
    }

    void Serialization::LoadRenderSettings()
    {
        // если данные о настройках не сериализованы - ничего не пишем
        if (!base_.has_render_settings()) 
        {
            return;
        }

        auto& p_settings = base_.render_settings();

        renderer::RenderSettings settings;

        settings.width = MakePoint(p_settings.size()).x;
        settings.height = MakePoint(p_settings.size()).y;

        settings.padding = p_settings.padding();

        settings.line_width = p_settings.line_width();
        settings.stop_radius = p_settings.stop_radius();

        settings.bus_label_font_size = p_settings.bus_label_font_size();
        settings.bus_label_offset = MakePoint(p_settings.bus_label_offset());

        settings.stop_label_font_size = p_settings.stop_label_font_size();
        settings.stop_label_offset = MakePoint(p_settings.stop_label_offset());

        settings.underlayer_color = MakeColor(p_settings.underlayer_color());
        settings.underlayer_width = p_settings.underlayer_width();

        auto color_pallete_count = p_settings.color_palette_size();
        settings.color_palette.clear();
        settings.color_palette.reserve(color_pallete_count);
        for (int i = 0; i < color_pallete_count; ++i) {
            settings.color_palette.push_back(MakeColor(p_settings.color_palette(i)));
        }
        map_renderer_.SetRenderSettings(settings);        
    }

    //------------------------Сейв настроек Маршрутизатора-------------------
    void Serialization::SaveRouteSettings()
    {
        auto settings = transport_router_.GetRouteSettings();

        base_.mutable_router_settings()->set_bus_velocity(settings.velocity);
        base_.mutable_router_settings()->set_bus_wait_time(settings.wait_time);
    }

    //-----------------------Сейф мапы (имя_оставноки;ее_айди)--------------
    void Serialization::SaveIdAndStopnames()
    {
        auto IdToStopname = transport_router_.GetStopnameToVertexId();
        for (auto& idtostp : IdToStopname)
        {
            base_.mutable_database()->mutable_id_by_stopname()->insert({ std::string(idtostp.first), static_cast<int32_t>(idtostp.second) });
        }
    }

    //------------------------Сейв внутренней инфы графа(не дякулы)-------------------
    void Serialization::SaveGraph()
    {        
        auto& graph = transport_router_.GetGraph();
            for (auto& edge : graph.GetEdges())
            {
                graph_proto::Edge temp_edge;
                temp_edge.set_from(edge.from);
                temp_edge.set_to(edge.to);
                temp_edge.mutable_weight()->set_bus_name(edge.weight.bus_name.data());                
                temp_edge.mutable_weight()->set_span_count(edge.weight.span_count);
                temp_edge.mutable_weight()->set_total_time(edge.weight.total_time);                
                *base_.mutable_database()->mutable_route_graph()->add_edges() = temp_edge;
            }

            for (auto& list : graph.GetIncidenceLists())
            {
                graph_proto::IncidenceList temp_list;
                for (auto edge_id : list) {
                    temp_list.add_edge_id(edge_id);
                }
                *base_.mutable_database()->mutable_route_graph()->mutable_incidence_list()->Add() = temp_list;
            }
    }

    //------------------------Сейв внутренней инфы маршрутизатора-------------------
    void Serialization::SaveRouter()
    {
        auto& router = transport_router_.GetRouter();
        for (auto& vector : router.get()->GetRoutesInternalData()) 
        {
            transport_router_proto::RoutesInternalDataVector temp_data;
            for (auto& data : vector) 
            {
                transport_router_proto::RoutesInternalData internal_data;
                transport_router_proto::RoutesInternalDataOptional optional_data;
                if (data.has_value()) 
                {
                    internal_data.mutable_rid_weight()->set_bus_name(std::string( data.value().weight.bus_name));
                    internal_data.mutable_rid_weight()->set_span_count(data.value().weight.span_count);
                    internal_data.mutable_rid_weight()->set_total_time(data.value().weight.total_time);
                    if (data->prev_edge.has_value())
                    {
                        internal_data.set_edgeid(*data->prev_edge);
                    }
                    else 
                    {
                        internal_data.set_nullopt(true);
                    }
                    *optional_data.mutable_data() = internal_data;
                }
                else
                {
                    optional_data.set_nullopt(true);
                }
                *temp_data.add_data() = optional_data;
            }
            *base_.mutable_routerdb()->mutable_routes_internal_data()->Add() = temp_data;
        }
    }    

    //------------------------Загрузка настроек Маршрутизатора-------------------
    void Serialization::LoadRouterSettings()
    {
        transport_router::TransportRouter::RouteSettings settings;
        settings.velocity = base_.router_settings().bus_velocity();
        settings.wait_time = base_.router_settings().bus_wait_time();
        transport_router_.SetRouteSettings(settings);        
    }

    //------------------------Загрузка мап остановок-------------------
    void Serialization::LoadRouterIdsAndStops()
    {       
        std::unordered_map<size_t, const domain::Stop*> stopname_by_id;
        std::unordered_map<std::string_view, size_t> id_by_stopname;

        for (const auto& [name, id] : base_.database().id_by_stopname())
        {
            const auto& stop = transport_catalogue_.GetStopptr(name);          

            stopname_by_id.emplace(id, stop);
            id_by_stopname.emplace(name, id);
        }

        transport_router_.SetStopnameToVertexId(std::move(id_by_stopname));
        transport_router_.SetVertexIdToStopname(std::move(stopname_by_id));
    }

    //------------------------Загрузка графа(не дякулы)-------------------
    void Serialization::LoadGraph()
    {
        std::vector<std::vector<size_t>> incidence_lists;
        for (auto& list : base_.database().route_graph().incidence_list())
        {
            std::vector<size_t> temp_data;
            for (auto data : list.edge_id()) 
            {
                temp_data.push_back(data);
            }
            incidence_lists.push_back(temp_data);
        }

        std::vector<graph::Edge<transport_router::RouteWeight>> edges;
        for (auto& edge : base_.database().route_graph().edges()) 
        {
            graph::Edge<transport_router::RouteWeight> temp_edge;

            temp_edge.from = edge.from();
            temp_edge.to = edge.to();
            temp_edge.weight.bus_name = edge.weight().bus_name();
            temp_edge.weight.span_count = edge.weight().span_count();
            temp_edge.weight.total_time = edge.weight().total_time();          

            edges.push_back(temp_edge);
        }

        auto graph = graph::DirectedWeightedGraph<transport_router::RouteWeight>(std::move(edges), std::move(incidence_lists));

        transport_router_.SetGraph(std::move(graph));
    }

    //------------------------Загрузка Маршрутизатора-------------------
    void Serialization::LoadRouter()
    {
        auto router = std::make_unique<graph::Router<transport_router::RouteWeight>>
            (graph::Router<transport_router::RouteWeight>(base_.routerdb(), transport_router_.GetGraph()));
        transport_router_.SetRouter(std::move(router));
    }

    //-----------------Вспомогательные функции-------------------------------------
    svg_serialize::Point Serialization::MakeProtoPoint(const svg::Point& point)
    {
        svg_serialize::Point result;
        result.set_x(point.x);
        result.set_y(point.y);
        return result;
    }

    svg::Point Serialization::MakePoint(const svg_serialize::Point& p_point)
    {
        svg::Point result;
        result.x = p_point.x();
        result.y = p_point.y();
        return result;
    }

    svg_serialize::Color Serialization::MakeProtoColor(const svg::Color& color)
    {
        svg_serialize::Color result;
        if (std::holds_alternative<std::string>(color))
        {
            result.set_string_color(std::get<std::string>(color));
        }
        else if (std::holds_alternative<svg::Rgb>(color))
        {
            auto& rgb = std::get<svg::Rgb>(color);
            auto p_color = result.mutable_rgb_color();
            p_color->set_r(rgb.red);
            p_color->set_g(rgb.green);
            p_color->set_b(rgb.blue);
        }
        else if (std::holds_alternative<svg::Rgba>(color))
        {
            auto& rgba = std::get<svg::Rgba>(color);
            auto p_color = result.mutable_rgba_color();
            p_color->set_r(rgba.red);
            p_color->set_g(rgba.green);
            p_color->set_b(rgba.blue);
            p_color->set_o(rgba.opacity);
        }
        return result;
    }

    svg::Color Serialization::MakeColor(const svg_serialize::Color& p_color)
    {
        svg::Color color;
        switch (p_color.color_case()) {
        case svg_serialize::Color::kStringColor:
            color = p_color.string_color();
            break;
        case svg_serialize::Color::kRgbColor:
        {
            auto& p_rgb = p_color.rgb_color();
            color = svg::Rgb(p_rgb.r(), p_rgb.g(), p_rgb.b());
        }
        break;
        case svg_serialize::Color::kRgbaColor:
        {
            auto& p_rgba = p_color.rgba_color();
            color = svg::Rgba(p_rgba.r(), p_rgba.g(), p_rgba.b(), p_rgba.o());
        }
        break;
        default:
            color = svg::NoneColor;
            break;
        }
        return color;
    }
}