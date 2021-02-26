#include "collect_dpt.hpp"
#include "dpt_thread_statistics.hpp"
#include "http_toolbox.hpp"
#include <sstream>
#include <string_view>
#include "string_toolbox.hpp"
#include <vector>

#define BOOST_JSON_STANDALONE
#include "boost/json/src.hpp"

namespace dpt {
std::vector<dpt::statistics> collect() {
    using namespace toolbox::http;

    session fourchannel_session {"a.4cdn.org"};
    request catalog_request = fourchannel_session.get("g/catalog.json");
    std::vector<dpt::statistics> threads;

    if (catalog_request.send()) {
        static std::vector<char> buffer;
        std::size_t n_read = catalog_request.read_to_dynamic_buffer(buffer);
        std::string_view catalog {buffer.data(), n_read};
        boost::json::value val = boost::json::parse(catalog);
        boost::json::array arr = val.get_array();
        for (const auto& page : arr) {
            for (const auto& thrd : page.at("threads").get_array()) {
                boost::json::object obj = thrd.get_object();
                if ((obj.if_contains("sub")) && (toolbox::string::starts_with(obj.at("sub").as_string(), "/dpt/"))) {
                    auto& dpt_thread = threads.emplace_back(obj.at("no").as_int64(), obj.at("sub").as_string(), obj.at("now").as_string());
                    request thread_request = fourchannel_session.get("g/thread/" + std::to_string(obj.at("no").as_int64()) + ".json");
                    if (thread_request.send()) {
                        static std::vector<char> thread_buffer;
                        std::size_t n_read = thread_request.read_to_dynamic_buffer(thread_buffer);
                        std::string_view thread_data {thread_buffer.data(), n_read};
                        boost::json::value val = boost::json::parse(thread_data);
                        boost::json::array arr = val.get_object().at("posts").get_array();
                        for (const auto& post : arr) {
                            boost::json::object obj = post.get_object();
                            if (obj.contains("com")) {
                                const std::string txt {obj.at("com").as_string()};
                                dpt_thread.add_post(txt);
                            }
                        }
                    }
                }
            }
        }
    }
    return std::move(threads);
}
} // namespace dpt
