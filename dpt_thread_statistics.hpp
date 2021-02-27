#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dpt {
struct statistics {
    using mentions_counter = std::map<std::string, std::size_t>;
    struct post {
        post(std::string_view&& text, bool quotes, bool quotes_op);
        std::string text;
        bool quotes;
        bool quotes_op;
    };

    statistics(unsigned int id, std::string_view&& title, std::string_view&& timestamp);

    const unsigned int id;
    const std::string title;
    const std::string timestamp;
    std::vector<post> posts;

    mentions_counter language_mentions;
    mentions_counter meme_posts;
    mentions_counter topic_discussions;
    mentions_counter insults;
    mentions_counter programming_jokes;
    mentions_counter buzzwords;

    std::size_t n_code_snippets;

    void add_post(std::string post);
    std::string thread_info_to_string() const;
};
} // namespace dpt
