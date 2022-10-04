#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include <optional>

using namespace transport_catalogue;

namespace request_h
{
    class RequestHandler
    {
    public:
        RequestHandler() = delete;

        RequestHandler(const transport_catalogue::Transport_catalogue& tc, const renderer::MapRenderer& renderer);

        // ���������� ���������� � �������� (������ Bus)
        std::optional<domain::StopsForBusResponse> GetBusStat(const std::string_view& bus_name) const;

        // ���������� ��������, ���������� �����
        const std::unordered_set<std::string_view> GetBusesByStop(const std::string_view& stop_name) const;

        // ���������� ����� � ���� ���������
        svg::Document RenderMap() const;

        void RenderMap(std::ostream& out);

    private:
        // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
        const transport_catalogue::Transport_catalogue& tc_;
        const renderer::MapRenderer& renderer_;
    };
}