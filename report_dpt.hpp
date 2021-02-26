#pragma once

#include "analyse_dpt.hpp"
#include <ostream>

namespace dpt {
void report(std::ostream& os, const dpt::statistics& stats);
} // namespace dpt
