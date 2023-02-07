
#include "transport_catalogue.h"
#include "json_reader.h"
#include "transport_router.h"

int main() 
{
    transport_catalogue::Transport_catalogue tr; 
    renderer::MapRenderer rend;
    transport_router::TransportRouter router(tr);
    request_h::RequestHandler reqhan(tr, rend, router);
    json_reader::JsonReader js(tr, reqhan, rend, router, std::cin);
    js.BeginToMakeBase();
    js.ResponsesToRequests(std::cout);
}