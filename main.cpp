#include "analyse_dpt.hpp"
#include "collect_dpt.hpp"
#include "dpt_thread_statistics.hpp"
#include "report_dpt.hpp"

#include <iostream>

int main() {
    auto thread_data = dpt::collect();

    for (auto& thread : thread_data) {
        dpt::analyse(thread);
        dpt::report(std::cout, thread);
    }
    return 0;
}
