#include <fstream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "transport_router.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    transport_catalogue::Transport_catalogue tr;
    renderer::MapRenderer rend;
    transport_router::TransportRouter router(tr);
    serialization::Serialization serializator(tr, rend, router);
    request_h::RequestHandler reqhan(serializator, tr, rend, router);
    json_reader::JsonReader js(serializator, tr, reqhan, rend, router);

    //std::ofstream ofs("C://Users//User//source//repos//Transport_catalogue//primer1.json");

    if (mode == "make_base"sv) {

        js.BeginToMakeBase(std::cin);
    }
    else if (mode == "process_requests"sv) {

        js.ReadRequests(std::cin);
        js.ResponsesToRequests(std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}

/*int main()
{
    transport_catalogue::Transport_catalogue tr;
    renderer::MapRenderer rend;
    transport_router::TransportRouter router(tr);
    serialization::Serialization serializator(tr, rend, router);
    request_h::RequestHandler reqhan(serializator, tr, rend, router);
    json_reader::JsonReader js(serializator, tr, reqhan, rend, router);
    //js.BeginToMakeBase(std::cin);
    js.ReadRequests(std::cin);
    js.ResponsesToRequests(std::cout);
}*/
