#include "domain.h"

std::size_t domain::detail::DistHasher::operator()(const std::pair<const Stop*, const Stop*> pairptr) const
{
    std::size_t ptr1hash = s_hasher_(pairptr.first);
    std::size_t ptr2hash = s_hasher_(pairptr.second);

    return ptr1hash * 37 + ptr2hash * 37 * 37;
}