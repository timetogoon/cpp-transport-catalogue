#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;
using namespace literals;

    void output::GetInfo(Transport_catalogue& name, std::istream& input, std::ostream& output)
    {
        size_t request;
        input >> request;
        string parsestring;
        getline(input, parsestring, '\n');
        for (int i = 0; i < request; ++i)
        {
            getline(input, parsestring, '\n');

            if (parsestring.find("Bus"s) != parsestring.npos) //вывод на запрос информации о маршруте
            {
                parsestring = parsestring.substr(parsestring.find_first_of(' ') + 1,
                    parsestring.find_last_not_of(' ') + 1);
                if (name.FindBus(parsestring).name.empty())
                {
                    output << "Bus "s << parsestring << ": not found"s << endl;
                }
                else
                {
                    auto result = name.GetBusInfo(parsestring);
                    output << "Bus "s << parsestring << ": "s << result.stopsnumber
                        << " stops on route, "s
                        << result.uniquestops << " unique stops, "s
                        << result.routelength << " route length, "s
                        << result.curvature << " curvature"s << endl;
                }
            }
            else if (parsestring.find("Stop"s) != parsestring.npos) //вывод на запрос информации об остановке
            {
                parsestring = parsestring.substr(5, parsestring.find(':'));
                if (name.FindStop(parsestring).name.empty())
                {
                    output << "Stop "s << parsestring << ": not found"s << endl;
                }
                else if (name.GetStopsForBuses(parsestring).size() == 0)
                {
                    output << "Stop " << parsestring << ": no buses" << std::endl;
                }
                else
                {
                    output << "Stop "s << parsestring << ": buses"s;
                    auto result = name.GetStopsForBuses(parsestring);
                    set<string_view> temp(result.begin(), result.end()); //для вывода автобусов в алфавитном порядке
                    for (auto& res : temp)
                    {
                        output << " "s << res;
                    }
                    output << endl;
                }
            }
            else
            {
                output << "incorrect request!"s;
            }
        }
    }

