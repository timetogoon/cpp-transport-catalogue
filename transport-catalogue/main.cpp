#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace transport_catalogue;

int main() 
{
    std::ofstream out("test.txt");
    primary::Transport_catalogue tr;
    input::parsing::AddToCatalogue(tr, std::move(input::detail::ParseRequests(std::cin)));
    output::GetInfo(tr, std::cin, std::cout);
}