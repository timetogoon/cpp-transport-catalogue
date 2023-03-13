#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"
#include <optional>

using namespace transport_catalogue;

namespace request_h
{
    class RequestHandler
    {
    public:
        RequestHandler() = delete;

        RequestHandler(const serialization::Serialization& serializator,
                       const transport_catalogue::Transport_catalogue& tc,
                       const renderer::MapRenderer& renderer,
                       const transport_router::TransportRouter& troute);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<domain::StopsForBusResponse> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::unordered_set<std::string_view> GetBusesByStop(const std::string_view& stop_name) const;

        // Возвращает карту в виде документа
        svg::Document RenderMap() const;

        // "Рисует" карту
        void RenderMap(std::ostream& out);
        
        // Поиск маршрута
        std::optional<domain::ReportRouter>BuildRoute(const std::string& from, const std::string& to) const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты" и не только...
        const serialization::Serialization& serializator_;
        const transport_catalogue::Transport_catalogue& tc_;
        const renderer::MapRenderer& renderer_;
        const transport_router::TransportRouter& troute_;
    };
}