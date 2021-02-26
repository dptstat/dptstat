#pragma once

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace toolbox {
namespace string {
namespace internal {
template <typename StringType = char> constexpr std::size_t
size_helper(StringType&& str) {
    return str.size();
}
template <typename StringType = char> constexpr std::size_t
size_helper(const char * const str) {
    return std::char_traits<char>::length(str);
}
template <typename StringType = char> constexpr std::size_t
size_helper(char) {
    return 1;
}
template <class CharT = char>
class string_view_helper : public std::basic_string_view<CharT> { /// unsafe, never use as return value
public:
    template <typename ... StringType> constexpr string_view_helper(StringType&& ... str) noexcept : std::basic_string_view<CharT>{std::forward<StringType>(str)...} {}
    template <typename StringType = CharT> constexpr string_view_helper(CharT&& str) noexcept : std::basic_string_view<CharT>{&str, 1u} {}
};
template <class CharT = char>
using string_view = internal::string_view_helper<std::decay_t<CharT>>;
} // namespace internal
template <typename CharT = char, typename StringType> constexpr bool
contains(internal::string_view<CharT>&& str, StringType&& substr) noexcept {
    return str.find(std::forward<StringType>(substr)) != std::string::npos;
}
template <typename CharT = char, typename StringType> constexpr std::size_t
count(internal::string_view<CharT>&& str, StringType&& substr) noexcept {
    std::size_t n {0};
    std::size_t p {0};
    do {
        p = str.find(std::forward<StringType>(substr), p);
        if (p != std::string::npos) {
            ++n;
            ++p;
        }
    } while(p != std::string::npos);
    return n;
}
template <typename CharT = char, typename StringType> constexpr bool
starts_with(internal::string_view<CharT>&& str, StringType&& substr) noexcept {
    return str.find(std::forward<StringType>(substr), 0) == 0;
}
template <typename CharT = char> constexpr bool
ends_with(internal::string_view<CharT>&& str, internal::string_view<CharT>&& substr) noexcept {
    auto str_n = str.size();
    auto substr_n = substr.size();
    return (str_n >= substr_n) && (str.compare(str_n - substr_n, substr_n, substr) == 0);
}
template <typename CharT = char> constexpr bool
is_numeric(internal::string_view<CharT>&& str_view) {
    return (!str_view.empty()) && (std::find_if(str_view.begin(), str_view.end(), [](char c){return !std::isdigit(c) && c != '.' && c != ','; }) == str_view.end());
}
template <typename StringType> constexpr StringType
rtrim(StringType str) {
    auto p = str.find_last_not_of(" \n\r\t");
    if (p != std::string::npos) {
        str.erase(p + 1);
    }
    return std::move(str);
}
template <typename StringType> constexpr std::decay_t<StringType>
ltrim(StringType&& str) {
    auto p = str.find_first_not_of(" \n\r\t");
    if (p != std::string::npos) {
        return std::move(str.substr(p));
    }
    return str;
}
template <typename BasicString> constexpr std::decay_t<BasicString>
trim(BasicString&& str) {
    return std::move(ltrim(std::move(rtrim(std::forward<BasicString>(str)))));
}
template <typename BasicString, typename StringType> constexpr std::decay_t<BasicString>
erase(BasicString str, StringType&& substr) {
    auto p = std::string::npos;
    while ((p = str.find(std::forward<StringType>(substr))) != std::string::npos) {
        str.erase(p, internal::size_helper<StringType>(std::forward<StringType>(substr)));
    }
    return std::move(str);
}
template <typename BasicString, typename StringType, typename ReplaceType> constexpr std::decay_t<BasicString>
replace(BasicString str, StringType&& substr, ReplaceType&& repl) {
    std::size_t p = 0;
    while ((p = str.find(std::forward<StringType>(substr), p)) != std::string::npos) {
        str.replace(p, internal::size_helper<StringType>(std::forward<StringType>(substr)), std::forward<ReplaceType>(repl));
        p += internal::size_helper<ReplaceType>(repl);
    }
    return std::move(str);
}
template <typename Container = std::vector<std::string>, typename BasicString, typename Delimiter> constexpr Container
split(BasicString str, Delimiter&& delimiter) {
    Container parts{};
    auto p = std::string::npos;
    while ((p = str.find(delimiter)) != std::string::npos) {
        parts.emplace(parts.end(),str.substr(0, p));
        str.erase(0, p + internal::size_helper<Delimiter>(std::forward<Delimiter>(delimiter)));
    }
    parts.emplace(parts.end(), str.substr((p == std::string::npos) ? 0 : p));
    return std::move(parts);
}
template <typename BasicString = std::string, typename Delimiter, typename Container> constexpr BasicString
implode(Container&& container, Delimiter&& delimiter) {
    BasicString str{};
    for (const auto& part : container) {
        str += (part + delimiter);
    }
    if (!str.empty()) {
        str.pop_back();
    }
    return std::move(str);
}
} // namespace string
} // namespace toolbox
