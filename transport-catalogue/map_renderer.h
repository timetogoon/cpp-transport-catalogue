#pragma once

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>

namespace renderer {

    struct RenderSettings
    {
        double width = 0.0;
        double height = 0.0;

        double padding = 0.0;

        double line_width = 0.0;
        double stop_radius = 0.0;

        int bus_label_font_size = 0;
        svg::Point bus_label_offset;

        int stop_label_font_size = 0;
        svg::Point stop_label_offset;

        svg::Color underlayer_color;
        double underlayer_width = 0.0;

        std::vector<svg::Color> color_palette;
    };

    class MapDraw : public svg::Drawable
    {
    public:
        MapDraw(const RenderSettings& settings, const std::deque<domain::Bus>& buses);

        void Draw(svg::ObjectContainer& container) const override;
        
    private:
        std::vector<geo::Coordinates> geo_coords_;
        std::map<std::string, svg::Point> stop_earth_coord_;
        const RenderSettings& settings_;
        const std::deque<domain::Bus>& buses_;
        void DrawLines(svg::ObjectContainer& container) const;
        void DrawRouteNames(svg::ObjectContainer& container) const;
        void DrawCirclesStops(svg::ObjectContainer& container) const;
        void DrawStopsNames(svg::ObjectContainer& container) const;
    };
    
    class MapRenderer final
    {
    public:
        MapRenderer() = default;

        void SetRenderSettings(const RenderSettings& settings);

        MapDraw GetPictures(const std::deque<domain::Bus>& buses) const;

    private:
        RenderSettings settings_;
    };
}