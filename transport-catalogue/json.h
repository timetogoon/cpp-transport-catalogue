#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : public std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
    {
    public:
        using variant::variant;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Dict& AsMap() const;
        const Array& AsArray() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const variant& GetValue() const {
            return *this;
        }
    };

    inline bool operator==(const Node& lhs, const Node& rhs)
    {
        return lhs.GetValue() == rhs.GetValue();
    }

    inline bool operator!=(const Node& lhs, const Node& rhs)
    {
        return !(lhs == rhs);
    }

    class Document final
    {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;
        
    private:
        Node root_;             
    };    

    inline bool operator==(const Document& lhs, const Document& rhs)
    {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs)
    {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    struct PrintContext
    {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        // ¬озвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const
        {
            return { out, indent_step, indent_step + indent };
        }
    };

}  // namespace json