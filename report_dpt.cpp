#include <iomanip>
#include <map>
#include <ostream>
#include "report_dpt.hpp"
#include <sstream>
#include <string>
#include <string_view>

#include <iostream>

namespace {
template <std::size_t width = 50, std::size_t margin = 8>
struct horizontal_table_buffer {
    int table_count{0};
    std::size_t index{0};
    std::size_t actual_width{width};
    std::vector<std::string> lines{};

    void write_table_ln(std::string_view line) {
        if (lines.size() == index) {
            lines.emplace_back((table_count - 1) * (width + margin), ' ');
        }
        lines[index] += line;
        ++index;
    }
    void new_table() {
        table_count++;

        index = 0;
        for (const auto& line : lines) {
            actual_width = std::max(width, line.size());
        }
        for (auto& line : lines) {
            line += std::string((std::max((table_count - 1) * width, actual_width) + margin) - line.size(), ' ');
        }
    }
    void to_stream(std::ostream& os) const {
        for (const auto& line : lines) {
            os << line << std::endl;
        }
    }
};

void language_mentions_overview(std::ostream& os, const dpt::statistics& stats) {
    os << "Languages discussed by post count" << std::endl;

    const std::size_t index_width = 5;
    const std::size_t index_numbers = 4;
    const std::size_t column_width = 7;
    const std::size_t column_height = 20;
    const std::size_t bar_width = 2;
    const char bar_character = 178;
    const std::size_t name_rows = 2;
    std::size_t max_mentions = 0;
    for (const auto& [language, mentions] : stats.language_mentions) {
        max_mentions = std::max(max_mentions, mentions);
    }
    const float scaling_factor = static_cast<float>(max_mentions) / static_cast<float>(column_height);

    os << std::endl;
    os << std::setw(index_width) << " ";
    for (const auto& [language, mentions] : stats.language_mentions) {
        if (mentions == max_mentions) {
            os << std::left << std::setw(column_width) << mentions;
        } else {
            os << std::left << std::setw(column_width) << " ";
        }
    }
    os << std::endl;

    for (std::size_t i = column_height; i > 0; --i) {
        const std::size_t min_mentions = static_cast<float>(i) * scaling_factor;
        const std::size_t next_min_mentions = static_cast<float>(i-1) * scaling_factor;
        if ((i % (column_height / index_numbers)) == 0) {
            os << std::right << std::setw(3) << min_mentions;
        } else {
            os << std::right << std::setw(3) << " ";
        }
        os << std::right << std::setw(index_width - 3) << "| ";

        for (const auto& [language, mentions] : stats.language_mentions) {
            if (mentions >= min_mentions) {
                os << std::left << std::setw(column_width) << std::string(bar_width, bar_character);
            } else if (mentions >= next_min_mentions) {
                os << std::left << std::setw(column_width) << mentions;
            } else {
                os << std::left << std::setw(column_width) << " ";
            }
        }
        os << std::endl;
    }

    // what is this even im too dumb for this shit
    for (int i = name_rows - 1; i >= 0; --i) {
        os << std::right << std::setw(index_width) << "| ";
        int pos = (name_rows - i) - 1;
        int j = 1;
        int overflow = 0;
        for (const auto& [language, mentions] : stats.language_mentions) {
            if (((j + i) % name_rows) == 0) {
                os << std::string((pos * column_width) + overflow, ' ');
                os << std::left << std::setw(column_width) << language;
                int n_characters_left = (column_width * name_rows) - (std::max(language.size(), column_width) + (pos * column_width));
                if (n_characters_left > 0) {
                    os << std::string(n_characters_left, ' ');
                    overflow = 0;
                } else {
                    overflow = n_characters_left;
                }
            }
            ++j;
        }
        os << std::endl;
    }
    os << std::string((stats.language_mentions.size() * column_width + index_width), '-') << std::endl;
}
void basic_table_overview(horizontal_table_buffer<>& buffer, const dpt::statistics::mentions_counter& table, std::string_view header, std::string_view count_label) {
    if (!table.empty()) {
        buffer.new_table();
        buffer.write_table_ln(header);
        buffer.write_table_ln("");

        std::size_t column_width = 0;
        for (const auto& [key, mentions] : table) {
            column_width = std::max(key.size(), column_width);
        }
        for (const auto& [key, mentions] : table) {
            std::stringstream ss;
            ss << "  " << std::setw(column_width) << std::left << key << " : " << count_label << " " << mentions << " time(s).";
            buffer.write_table_ln(ss.str());
        }
    }
}
} // namespace

namespace dpt {
void report(std::ostream& os, const dpt::statistics& stats) {
    os << "Thread statistics" << std::endl
       << stats.thread_info_to_string() << std::endl
       << std::endl;
    language_mentions_overview(os, stats);
    os << std::endl;
    os << "Actual number of code snippets posted: " << stats.n_code_snippets << std::endl
       << std::endl;
    {
        horizontal_table_buffer buffer{};
        basic_table_overview(buffer, stats.topic_discussions, "Topics discussed", "discussed");
        basic_table_overview(buffer, stats.meme_posts, "Irrelevant posts", "posted");
        basic_table_overview(buffer, stats.buzzwords, "Buzzwords", "counted");
        buffer.to_stream(os);
    }
    os << std::endl;
    {
        horizontal_table_buffer buffer{};
        basic_table_overview(buffer, stats.insults, "Groups insulted", "insulted");
        basic_table_overview(buffer, stats.programming_jokes, "Other statistics", "declared");
        buffer.to_stream(os);
    }
    os << std::endl;
    os << std::string(50, '_') << std::endl;
    os << std::endl;
}
} // namespace dpt
