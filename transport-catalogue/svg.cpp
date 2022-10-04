#include "svg.h"

namespace svg {

    using namespace std;
    using namespace std::literals;

    //Вывод цвета

    ostream& operator<<(ostream& out, Rgb rgb)
    {
        out << "rgb(" << int(rgb.red) << "," << int(rgb.green) << "," << int(rgb.blue) << ")";
        return out;
    }

    ostream& operator<<(ostream& out, Rgba rgba)
    {
        out << "rgba(" << int(rgba.red) << "," << int(rgba.green) << "," << int(rgba.blue) << "," << rgba.opacity << ")"; ;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, Color color)
    {
        std::visit(OstreamColorPrinter{ out }, color);
        return out;
    }

    // Вывод LineCup и LineJoin

    ostream& operator<<(ostream& out, StrokeLineCap line_cup)
    {
        if (line_cup == StrokeLineCap::BUTT) {
            out << "butt";
        }
        if (line_cup == StrokeLineCap::ROUND) {
            out << "round";
        }
        if (line_cup == StrokeLineCap::SQUARE) {
            out << "square";
        }
        return out;
    }

    ostream& operator<<(ostream& out, StrokeLineJoin line_join)
    {
        if (line_join == StrokeLineJoin::ARCS) {
            out << "arcs";
        }
        if (line_join == StrokeLineJoin::BEVEL) {
            out << "bevel";
        }
        if (line_join == StrokeLineJoin::MITER) {
            out << "miter";
        }
        if (line_join == StrokeLineJoin::MITER_CLIP) {
            out << "miter-clip";
        }
        if (line_join == StrokeLineJoin::ROUND) {
            out << "round";
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << endl;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point)
    {
        to_point_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (auto& point : to_point_)
        {
            if (first == true)
            {
                out << point.x << ","sv << point.y;
                first = false;
            }
            else
            {
                out << " "sv << point.x << ","sv << point.y;
            }
        }
        out << "\"";
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos)
    {
        pos_ = move(pos);
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        size_ = move(size);
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y
            << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
            << "\" font-size=\""sv << size_ << "\""sv;
        if (font_family_)
        {
            out << " font-family=\""sv << font_family_.value() << "\""sv;
        }
        if (font_weight_)
        {
            out << " font-weight=\""sv << font_weight_.value() << "\""sv;
        }
        out << ">"sv;
        out << EscapeText(data_);
        out << "</text>"sv;
    }

    string Text::EscapeText(const string& text) {
        unordered_map<char, string> characters = { {34,"&quot;"s},
                                                            {38,"&amp;"s},
                                                            {39,"&apos;"s},
                                                            {60,"&lt;"s},
                                                            {62,"&gt;"s} };
        string result;
        for (auto c : text) {
            auto iter = characters.find(c);
            if (iter != characters.end()) {
                result += iter->second;
            }
            else {
                result += c;
            }
        }
        return result;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const
    {
        RenderContext ctx(out, 2, 2);

        ctx.out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        ctx.out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }

        ctx.out << "</svg>"sv;
    }

}  // namespace svg