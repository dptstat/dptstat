#include "dpt_thread_statistics.hpp"
#include <sstream>
#include <string>
#include "string_toolbox.hpp"
#include <string_view>
#include <utility>
#include <vector>

namespace dpt {
statistics::statistics(unsigned int id, std::string_view&& title, std::string_view&& timestamp)
: id{id}, title{title}, timestamp{timestamp}, posts{},
  language_mentions{}, meme_posts{}, topic_discussions{}, insults{}, programming_jokes{}, buzzwords{},
  n_code_snippets{0} {}

statistics::post::post(std::string_view&& text, bool quotes, bool quotes_op)
: text{text}, quotes{quotes}, quotes_op{quotes_op} {}

void statistics::add_post(std::string post) {
    static const std::vector<std::pair<std::string, std::string>> to_erase = {
        {"<a href=\"#p", "</a>"}, // quotelink
        {"<a href=\"/g", "</a>"}  // threadlink
    };

    bool quotes = false;
    bool quotes_op = false;
    for (const auto& [begin_segment, end_segment] : to_erase) {
        std::size_t p = std::string::npos;
        while ((p = post.find(begin_segment)) != std::string::npos) {
            quotes = true;
            if (post.find("(OP)" + end_segment, p) != std::string::npos) {
                quotes_op = true;
            }
            auto p_end = post.find(end_segment) + end_segment.size();
            if (p_end != std::string::npos) {
                post.erase(p, p_end - p);
            } else {
                break;
            }
        }
    }
    post = toolbox::string::replace(std::move(post), "<span class=\"quote\">&gt;", " "); // greentext
    post = toolbox::string::replace(std::move(post), "</span>", " ");
    post = toolbox::string::replace(std::move(post), "<br>", " ");
    post = toolbox::string::replace(std::move(post), "&#039;", "'");
    post = toolbox::string::replace(std::move(post), "&quot;", "\"");
    post = toolbox::string::trim(std::move(post));

    posts.emplace_back(post, quotes, quotes_op);
}

std::string statistics::thread_info_to_string() const {
    std::stringstream ss;
    ss << timestamp << " - /g/thread/" << id << " - " << title;
    return std::move(ss.str());
}
} // namespace dpt
