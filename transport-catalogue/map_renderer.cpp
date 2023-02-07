#include "map_renderer.h"

using namespace renderer;

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

//----------------Преобразование координат в экранные--------------------------
class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

//----------------Конструктор класса рисования--------------------------
MapDraw::MapDraw(const RenderSettings& settings, const std::deque<domain::Bus>& buses)
    : settings_(settings),
    buses_(buses)
{
    for (auto const& bus : buses)
    {
        auto const& stops = bus.buses_stops;

        for (auto const& stop : stops)
        {
            geo_coords_.push_back(stop->coord);
        }
    }

    const SphereProjector proj
    {
      geo_coords_.begin(), geo_coords_.end(), settings_.width, settings_.height, settings_.padding
    };

    for (auto const& bus : buses)
    {
        auto const& stops = bus.buses_stops;

        for (auto const& stop : stops)
        {
            stop_earth_coord_[stop->name] = proj(stop->coord);
        }
    }
}

//----------------Рисуем линии маршрутов--------------------------
void MapDraw::DrawLines(svg::ObjectContainer& container) const
{
    auto sortbuses = buses_;
    std::sort(sortbuses.begin(), sortbuses.end(),
        [](const domain::Bus& lhs, const domain::Bus& rhs)
        { return lhs.name < rhs.name; });

    size_t busindex = 0;
    for (auto const& bus : sortbuses)
    {
        svg::Polyline line;
        line.SetStrokeColor(settings_.color_palette[busindex % settings_.color_palette.size()])
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetFillColor(svg::NoneColor)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        auto const& stops = bus.buses_stops;
        busindex++;     

        for (auto const& stop : stops)
        {
            line.AddPoint(stop_earth_coord_.at(stop->name));
        }
        if (!bus.ring)
        {
            for (auto it = bus.buses_stops.rbegin() + 1u; it != bus.buses_stops.rend(); it++)
            {
                line.AddPoint(stop_earth_coord_.at((**it).name));
            }
        }
        container.Add(std::move(line));
    }
}

//----------------Рисуем имена маршрутов--------------------------
void MapDraw::DrawRouteNames(svg::ObjectContainer& container) const 
{
    auto sortbuses = buses_;
    std::sort(sortbuses.begin(), sortbuses.end(),
        [](const domain::Bus& lhs, const domain::Bus& rhs)
        { return lhs.name < rhs.name; });
    size_t busindex = 0;
    for (auto const& bus : sortbuses)
    {
           const svg::Text base_text =
            svg::Text()
            .SetPosition(stop_earth_coord_.at(bus.buses_stops.front()->name))
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetData(bus.name);
            container.Add(svg::Text{ base_text }
            .SetStrokeColor(settings_.underlayer_color)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)            
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            container.Add(svg::Text{ base_text }
                .SetFillColor(settings_.color_palette[busindex % settings_.color_palette.size()]));        
        if (!bus.ring && bus.buses_stops.back() != bus.buses_stops.front())
        {
            const svg::Text base_text =
                svg::Text()
                .SetPosition(stop_earth_coord_.at(bus.buses_stops.back()->name))
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(bus.name);
            container.Add(svg::Text{ base_text }
                .SetStrokeColor(settings_.underlayer_color)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            container.Add(svg::Text{ base_text }
            .SetFillColor(settings_.color_palette[busindex % settings_.color_palette.size()]));
        }
        busindex++;
    }
}

//----------------Рисуем остановки маршрутов--------------------------
void MapDraw::DrawCirclesStops(svg::ObjectContainer& container) const
{
    for (auto const& [name, coords] : stop_earth_coord_)
    {
        container.Add(svg::Circle()
            .SetCenter(coords)
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white"s));
    }
}

//----------------Рисуем имена остановок маршрутов--------------------------
void MapDraw::DrawStopsNames(svg::ObjectContainer& container) const
{
    for (auto const& [name, coords] : stop_earth_coord_)
    {
        const svg::Text base_text =
            svg::Text()
            .SetPosition(coords)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetData(name);
        container.Add(svg::Text{ base_text }
            .SetStrokeColor(settings_.underlayer_color)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        container.Add(svg::Text{ base_text }
        .SetFillColor("black"s));        
    }
}

//----------------Рисуем--------------------------
void MapDraw::Draw(svg::ObjectContainer& container) const
{
    DrawLines(container);
    DrawRouteNames(container);
    DrawCirclesStops(container);
    DrawStopsNames(container);
}

//----------------Установка настроек рисования--------------------------
void MapRenderer::SetRenderSettings(const RenderSettings& settings)
{
    settings_ = settings;
}

//----------------Вызываем класс рисования--------------------------
MapDraw MapRenderer::GetPictures(const std::deque<domain::Bus>& buses) const
{
    return { settings_, buses };
}