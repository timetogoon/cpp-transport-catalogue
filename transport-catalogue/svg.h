#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <variant>

namespace svg {

    struct Rgb
    {
        Rgb() = default;
        Rgb(uint8_t red_, uint8_t green_, uint8_t blue_)
            : red(red_),
            green(green_),
            blue(blue_)
        {
        }

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba
    {
        Rgba() = default;
        Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_)
            : red(red_),
            green(green_),
            blue(blue_),
            opacity(opacity_)
        {
        }

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ "none" };

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, Rgb rgb);

    std::ostream& operator<<(std::ostream& out, Rgba rgba);

    std::ostream& operator<<(std::ostream& out, Color color);

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cup);

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

    struct OstreamColorPrinter
    {
        std::ostream& out;

        void operator() (std::monostate) const
        {
            out << "none";
        }
        void operator() (Rgb rgb) const
        {
            out << rgb;
        }
        void operator() (Rgba rgba) const
        {
            out << rgba;
        }
        void operator() (std::string color) const
        {
            out << color;
        }
    };

    template <typename Owner>
    class PathProps
    {
    public:
        Owner& SetFillColor(Color color);

        Owner& SetStrokeColor(Color color);

        Owner& SetStrokeWidth(double width);

        Owner& SetStrokeLineCap(StrokeLineCap line_cap);

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {

                out << " fill=\""sv;
                std::visit(OstreamColorPrinter{ out }, *fill_color_);
                out << "\""sv;;
            }

            if (stroke_color_)
            {
                out << " stroke=\""sv;
                std::visit(OstreamColorPrinter{ out }, *stroke_color_);
                out << "\""sv;
            }

            if (width_)
            {
                out << " stroke-width=\"" << *width_ << "\"";
            }

            if (line_cap_)
            {
                out << " stroke-linecap=\"" << *line_cap_ << "\"";
            }

            if (line_join_)
            {
                out << " stroke-linejoin=\"" << *line_join_ << "\"";
            }
        }

    private:
        Owner& AsOwner()
        {
            // static_cast ��������� ����������� *this � Owner&,
            // ���� ����� Owner � ��������� PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    template <typename Owner>
    Owner& PathProps<Owner>::SetFillColor(Color color)
    {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeColor(Color color)
    {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeWidth(double width)
    {
        width_ = std::move(width);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap)
    {
        line_cap_ = std::move(line_cap);
        return AsOwner();
    }

    template <typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join)
    {
        line_join_ = std::move(line_join);
        return AsOwner();
    }

    struct Point
    {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y)
        {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
     * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
     */
    struct RenderContext
    {
        RenderContext(std::ostream& out)
            : out(out)
        {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const
        {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * ����������� ������� ����� Object ������ ��� ���������������� ��������
     * ���������� ����� SVG-���������
     * ��������� ������� "��������� �����" ��� ������ ����������� ����
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * ����� Circle ���������� ������� <circle> ��� ����������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps <Circle>
    {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = { 0.0 , 0.0 };
        double radius_ = 1.0;
    };

    /*
     * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps <Polyline>
    {
    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);

        //������ ������ � ������, ����������� ��� ���������� �������� <polyline>
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> to_point_;
    };

    class ObjectContainer
    {
    public:
        /*
         ����� Add ��������� � svg-�������� ����� ������-��������� svg::Object.
        */
        template <typename Obj>
        void Add(Obj obj);

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;
    };

    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    /*
     * ����� Text ���������� ������� <text> ��� ����������� ������
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps <Text>
    {
    public:
        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(Point pos);

        // ����� �������� ������������ ������� ����� (�������� dx, dy)
        Text& SetOffset(Point offset);

        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);

        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(std::string font_family);

        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(std::string font_weight);

        // ����� ��������� ���������� ������� (������������ ������ ���� text)
        Text& SetData(std::string data);

        // ������ ������ � ������, ����������� ��� ���������� �������� <text>
    private:
        Point pos_ = { 0.0,0.0 };
        Point offset_ = { 0.0,0.0 };
        uint32_t size_ = 1u;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string data_ = "";
        void RenderObject(const RenderContext& context) const override;
        static std::string EscapeText(const std::string& text);
    };

    class Document final : public ObjectContainer
    {
    public:

        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;

        // ������ ������ � ������, ����������� ��� ���������� ������ Document
    private:
        std::vector<std::unique_ptr<Object>> objects_ = {};
    };

    template <typename Obj>
    void ObjectContainer::Add(Obj obj)
    {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

}  // namespace svg