#pragma once
#include "transport_catalogue.h"
#include <iomanip>

namespace output 
{
	void GetInfo(transport_catalogue::primary::Transport_catalogue& name, std::istream& input = std::cin, std::ostream& output = std::cout);
}