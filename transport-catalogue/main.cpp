#include "transport_catalogue.h"
#include "json_reader.h"

int main() 
{
    //std::ofstream out("test.txt");
    transport_catalogue::Transport_catalogue tr; 
    renderer::MapRenderer rend;
    request_h::RequestHandler reqhan(tr, rend);
    json_reader::JsonReader js(tr, reqhan, rend, std::cin);
    js.BeginToMakeBase();
    //reqhan.RenderMap();
    js.ResponsesToRequests(std::cout);
}