#include "json.h"
#include <cctype>
#include <iterator>

using namespace std;
using namespace string_literals;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadNull(istream& input)
        {
            string res;

            char c;

            for (int i = 0; i < 4; ++i) {
                if (input.get(c)) {
                    res += c;
                }
            }

            if (res != "null") {
                throw ParsingError("Failed to parse null node");
            }

            return Node();
        }

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input)
        {
            string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input]
            {
                parsed_num += static_cast<char>(input.get());
                if (!input)
                {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char]
            {
                if (!isdigit(input.peek()))
                {
                    throw ParsingError("A digit is expected"s);
                }
                while (isdigit(input.peek()))
                {
                    read_char();
                }
            };

            if (input.peek() == '-')
            {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0')
            {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else
            {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.')
            {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E')
            {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-')
                {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try
            {
                if (is_int)
                {
                    // Сначала пробуем преобразовать строку в int
                    try
                    {
                        return std::stoi(parsed_num);
                    }
                    catch (...)
                    {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...)
            {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input)
        {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true)
            {
                if (it == end)
                {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"')
                {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end)
                    {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char)
                    {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r')
                {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else
                {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return s;
        }

        Node LoadBool(istream& input)
        {
            std::string res;
            char c;

            c = static_cast<char>(input.peek());

            auto length = c;

            if (c == 't')
            {
                length = 4;
            }
            else
            {
                length = 5;
            }

            for (int i = 0; i < length; ++i) {
                if (input.get(c)) {
                    res += c;
                }
            }

            if (res == "true"sv)
            {
                return Node(true);
            }
            else if (res == "false"sv)
            {
                return Node(false);
            }
            else
            {
                throw ParsingError("Failed to parse bool node");
            }
        }

        Node LoadArray(istream& input)
        {
            Array result;

            for (char c; input >> c && c != ']';)
            {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (!input) {
                throw ParsingError("Array parsing error"s);
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input)
        {
            Dict result;

            for (char c; input >> c && c != '}';)
            {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (!input) {
                throw ParsingError("Array parsing error"s);
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input)
        {
            char c;
            input >> c;

            if (c == '[')
            {
                return LoadArray(input);
            }
            else if (c == 'n')
            {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == '{')
            {
                return LoadDict(input);
            }
            else if (c == '"')
            {
                return LoadString(input);
            }
            else if (isdigit(c) || c == '-')
            {
                input.putback(c);
                auto num = LoadNumber(input);
                if (holds_alternative<double>(num))
                {
                    return Node(get<double>(num));
                }
                else
                {
                    return Node(get<int>(num));
                }
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else
            {
                throw ParsingError("Failed to parse document"s);
            }
        }

    }  // namespace

    //------------------Проверка типов--------------------------


    bool Node::IsInt() const
    {
        return holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const
    {
        return holds_alternative<int>(*this) || holds_alternative<double>(*this);
    }

    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(*this);
    }

    bool Node::IsBool() const
    {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsString() const
    {
        return holds_alternative<string>(*this);
    }

    bool Node::IsNull() const
    {
        return holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const
    {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(*this);
    }
    
    //-------------------Возврат значений типов-----------------

    int Node::AsInt() const
    {
        if (!IsInt())
        {
            throw logic_error("Not a integer"s);
        }

        return get<int>(*this);
    }

    bool Node::AsBool() const
    {
        if (!IsBool())
        {
            throw logic_error("Not a bool"s);
        }

        return get<bool>(*this);
    }

    double Node::AsDouble() const
    {
        if (!IsDouble() && !IsPureDouble())
        {
            throw logic_error("Not a double"s);
        }

        if (IsInt())
        {
            return static_cast<double> (get<int>(*this));
        }

        return get<double>(*this);
    }

    const string& Node::AsString() const
    {
        if (!IsString())
        {
            throw logic_error("Not a string"s);
        }

        return get<string>(*this);
    }

    const Dict& Node::AsMap() const
    {
        if (!IsMap())
        {
            throw logic_error("Not a dictonary"s);
        }

        return get<Dict>(*this);
    }

    const Array& Node::AsArray() const
    {
        if (!IsArray())
        {
            throw logic_error("Not a array"s);
        }

        return get<Array>(*this);
    }

    //---------------Загрузка и вывод документов-------------

    Document::Document(Node root)
        : root_(move(root))
    {
    }

    const Node& Document::GetRoot() const
    {
        return root_;
    }

    Document Load(istream& input)
    {
        return Document{ LoadNode(input) };
    }

    //Escape последовательности
    std::string AddEscapes(const std::string& str) {
        std::string result;
        for (auto c : str) {
            if (c == '\"') {
                result += "\\\"";
            }
            else if (c == '\r') {
                result += "\\r";
            }
            else if (c == '\n') {
                result += "\\n";
            }
            else if (c == '\\') {
                result += "\\\\";
            }
            else {
                result += c;
            }
        }
        return result;
    }

    template <typename Value>
    void PrintValue(const Value& value, PrintContext& ptx)
    {
        if (value.IsNull())
        {
            ptx.out << "null"sv;
        }
        else if (value.IsInt())
        {
            ptx.out << value.AsInt();
        }
        else if (value.IsString())
        {
            ptx.out << "\""sv << AddEscapes(value.AsString()) << "\""sv;
        }
        else if (value.IsDouble())
        {
            ptx.out << value.AsDouble();
        }
        else if (value.IsBool())
        {
            if (value.AsBool())
            {
                ptx.out << "true"sv;
            }
            else
            {
                ptx.out << "false"sv;
            }
        }
        else if (value.IsArray())
        {
            auto& res = value.AsArray();
            std::ostream& out = ptx.out;
            out << "[\n"sv;
            bool first = true;
            auto arrptx = ptx.Indented();
            for (auto i = 0; i < static_cast<int>(res.size()); i++)
            {
                if (first == true)
                {
                    PrintValue(res[i], ptx);
                    first = false;
                }
                else
                {
                    ptx.out << ",\n"sv;
                    PrintValue(res[i], arrptx);
                }
            }
            out.put('\n');
            ptx.Indented();
            out << "]"sv;
        }
        else if (value.IsMap())
        {
            auto res = value.AsMap();
            std::ostream& out = ptx.out;
            out << "{\n\""sv;
            bool firstprnt = true;
            auto mapptx = ptx.Indented();
            for (const auto& [key, value] : res)
            {
                if (firstprnt == true)
                {
                    out << key;
                    out << "\": "sv;
                    PrintValue(value, mapptx);
                    firstprnt = false;
                }
                else
                {
                    out << ",\n";
                    out << "\""sv;
                    out << key;
                    out << "\": "sv;

                    PrintValue(value, mapptx);
                }
            }
            out.put('\n');
            ptx.PrintIndent();
            out << "}"sv;
        }
    }

    void Print(const Document& doc, std::ostream& output)
    {
        PrintContext ptx{ output };
        PrintValue(doc.GetRoot(), ptx);
    }
}  // namespace json