#pragma once

#include "dpt_thread_statistics.hpp"
#include "http_toolbox.hpp"
#include <vector>

namespace dpt {
std::vector<dpt::statistics> collect();
} // namespace dpt
