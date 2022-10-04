#pragma once
#include "transport_catalogue.h"
#include <iomanip>

namespace output 
{
	void GetInfo(transport_catalogue::Transport_catalogue& name, std::istream& input = std::cin, std::ostream& output = std::cout);
}