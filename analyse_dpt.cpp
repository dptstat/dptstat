#include <algorithm>
#include "analyse_dpt.hpp"
#include <array>
#include "dpt_thread_statistics.hpp"
#include <map>
#include <string>
#include "string_toolbox.hpp"
#include <string_view>
#include <utility>
#include <vector>

namespace {
enum search_policy {
    policy_no_transform     = 1 << 0,
    policy_lowercase        = 1 << 1,
    policy_no_punctuation   = 1 << 2,
    policy_simple_count     = 1 << 3,
    policy_count_helper     = 1 << 4,
    policy_exact_match      = 1 << 5,
    policy_unique           = 1 << 6,
    policy_count_all        = 1 << 7
};
int policy_standard       = (policy_lowercase | policy_simple_count | policy_unique);
int policy_case_sensitive = (policy_no_punctuation | policy_count_helper | policy_unique);
int policy_substring_risk = (policy_lowercase | policy_no_punctuation | policy_count_helper | policy_unique);
int policy_single_letter  = (policy_no_transform | policy_count_helper | policy_unique);
int policy_sentence       = (policy_lowercase | policy_no_punctuation | policy_simple_count | policy_unique);
int policy_single_word    = (policy_lowercase | policy_no_punctuation | policy_exact_match | policy_unique);

struct search_value {
    search_value(std::string_view key, const std::vector<std::string>& tokens, int policies, const std::vector<std::string>& occurs_in)
    : key{key}, tokens{tokens}, policies{policies}, occurs_in{occurs_in} {}
    search_value(std::string_view key, const std::vector<std::string>& tokens, int policies)
    : search_value{key, tokens, policies, {}} {}

    const std::string key;
    const std::vector<std::string> tokens;
    const int policies;
    const std::vector<std::string> occurs_in;
};
using search_values = std::vector<search_value>;

namespace definitions {
const search_values programming_languages = {
    {"ALGOL"        , {"algol"                           }, policy_standard},
    {"ActionScript" , {"actionscript"                    }, policy_standard},
    {"Assembly"     , {"assembly", "assembler"           }, policy_standard},
    {"B#"           , {"b#"                              }, policy_standard},
    {"Bash"         , {"bash"                            }, policy_standard},
    {"Batch"        , {"batch"                           }, policy_standard},
    {"C++"          , {"c++", "cpp", "sepples", "seppels"}, policy_standard},
    {"C#"           , {"c#"                              }, policy_standard},
    {"Clojure"      , {"clojure"                         }, policy_standard, {"clojurescript"}},
    {"ClojureScript", {"clojurescript"                   }, policy_standard},
    {"COBOL"        , {"cobol"                           }, policy_standard},
    {"CoffeeScript" , {"coffeescript"                    }, policy_standard},
    {"CSS"          , {"css"                             }, policy_standard},
    {"DCL"          , {"dcl"                             }, policy_standard},
    {"Delphi"       , {"delphi"                          }, policy_standard},
    {"Elixir"       , {"elixir"                          }, policy_standard},
    {"Erlang"       , {"erlang"                          }, policy_standard},
    {"F#"           , {"f#"                              }, policy_standard},
    {"Forth"        , {"forth"                           }, policy_substring_risk},
    {"Fortran"      , {"fortran"                         }, policy_standard},
    {"GLSL"         , {"glsl"                            }, policy_standard},
    {"Golang"       , {"golang"                          }, policy_standard},
    {"Haskell"      , {"haskell"                         }, policy_standard},
    {"HolyC"        , {"holyc"                           }, policy_standard},
    {"HTML"         , {"html"                            }, policy_standard},
    {"J#"           , {"j#"                              }, policy_standard},
    {"J++"          , {"j++"                             }, policy_standard},
    {"Java"         , {"java"                            }, policy_standard, {"javascript"}},
    {"JavaScript"   , {"javascript", "js"                }, policy_substring_risk},
    {"Kotlin"       , {"kotlin"                          }, policy_standard},
    {"Lisp"         , {"lisp"                            }, policy_standard},
    {"Machine code" , {"machine code"                    }, policy_standard},
    {"MATLAB"       , {"matlab"                          }, policy_standard},
    {"XML"          , {"xml"                             }, policy_standard},
    {"Objective-C"  , {"objective-c", "object-c"         }, policy_standard},
    {"OCaml"        , {"ocaml"                           }, policy_standard},
    {"Opal"         , {"opal"                            }, policy_standard},
    {"Pascal"       , {"pascal"                          }, policy_standard},
    {"PHP"          , {"php"                             }, policy_standard},
    {"PostScript"   , {"postscript"                      }, policy_standard},
    {"PowerShell"   , {"powershell"                      }, policy_standard},
    {"Pro*C"        , {"pro*c"                           }, policy_standard},
    {"Python"       , {"python"                          }, policy_standard},
    {"R++"          , {"r++"                             }, policy_standard},
    {"Racket"       , {"racket"                          }, policy_standard},
    {"Ruby"         , {"ruby"                            }, policy_standard},
    {"Rust"         , {"rust"                            }, policy_standard},
    {"Scala"        , {"scala"                           }, policy_standard},
    {"Scheme"       , {"Scheme"                          }, policy_case_sensitive},
    {"Smalltalk"    , {"smalltalk"                       }, policy_standard},
    {"SQL"          , {"sql"                             }, policy_standard},
    {"Swift"        , {"swift"                           }, policy_standard},
    {"TypeScript"   , {"typescript"                      }, policy_standard},
    {"VHDL"         , {"vhdl"                            }, policy_standard},
    {"Zig"          , {"zig"                             }, policy_substring_risk},
    {"Lua"          , {"lua"                             }, policy_substring_risk},
    {"Ada"          , {"ada"                             }, policy_substring_risk},
    {"Nim"          , {"nim"                             }, policy_substring_risk},
    {"Perl"         , {"perl"                            }, policy_substring_risk},
    {"Unity"        , {"unity"                           }, policy_substring_risk},
    {"C"            , {"c", "C"                          }, policy_single_letter},
    {"B"            , {"B"                               }, policy_single_letter},
    {"D"            , {"D"                               }, policy_single_letter},
    {"J"            , {"J"                               }, policy_single_letter},
    {"R"            , {"R"                               }, policy_single_letter},
    {"BASIC"        , {"BASIC"                           }, policy_case_sensitive},
    {"Go"           , {"Go"                              }, policy_case_sensitive}
};
const search_values memes = {
    {"\"In Haskell, this is just ...\""                 , {"in haskell this is just"                  }, policy_sentence},
    {"\"In Lisp, this is just ...\""                    , {"in lisp this is just"                     }, policy_sentence},
    {"\"... is the most powerful programming language\"", {"is the most powerful programming language"}, policy_sentence},
    {"\"First for ...\""                                , {"first for"                                }, policy_sentence},
    {"\"nth for ...\""                                  , {"nth for"                                  }, policy_sentence},
    {"The word \"algorithm\" and nothing else"          , {"algorithm"                                }, policy_single_word}
};
const search_values topics = {
    {"SICP"                  , {"sicp"              }, policy_substring_risk},
    {"OOP/POO"               , {"oop", "poo"        }, policy_substring_risk},
    {"Functional programming", {"functional", "fp"  }, policy_substring_risk},
    {"Autism"                , {"autism", "autistic"}, policy_standard},
    {"Design patterns"       , {"design pattern"    }, policy_standard},
    {"Anime"                 , {"anime", "weeb"     }, policy_standard},
    {"CMake"                 , {"cmake"             }, policy_standard},
    {"Makefiles"             , {"makefile"          }, policy_standard},
    {"Monads"                , {"monad"             }, policy_standard},
    {"SOLID"                 , {"SOLID"             }, policy_case_sensitive},
    {"OpenGL"                , {"opengl"            }, policy_standard},
    {"Metaprogramming"       , {"metaprogramming"   }, policy_standard},
    {"Vulkan"                , {"vulkan"            }, policy_standard}
};
const search_values programming_jokes = {
    {"Fizz"       , {"fizz"       }, (policy_standard &~ policy_unique) | policy_count_all ,      {"fizzbuzz"}},
    {"Buzz"       , {"buzz"       }, (policy_standard &~ policy_unique) | policy_count_all ,      {"fizzbuzz"}},
    {"FizzBuzz"   , {"fizzbuzz"   }, (policy_standard &~ policy_unique) | policy_count_all},
    {"Foo"        , {"foo"        }, (policy_substring_risk &~ policy_unique) | policy_count_all, {"foobar"}},
    {"Bar"        , {"bar"        }, (policy_substring_risk &~ policy_unique) | policy_count_all, {"foobar"}},
    {"FooBar"     , {"foobar"     }, (policy_standard &~ policy_unique) | policy_count_all},
    {"Hello World", {"hello world"}, (policy_sentence &~ policy_unique) | policy_count_all}
};
const search_values insults = {
    {"Cniles"        , {"cnile", "c-nile"              }, policy_standard},
    {"Transsexuals"  , {"tranny", "trannie"            }, policy_standard},
    {"Gays"          , {"fag", "faggot", "gay"         }, policy_standard},
    {"Codemonkeys"   , {"codemonkey", "code-monkey"    }, policy_standard},
    {"Brainlets"     , {"brainlet", "retard"           }, policy_standard},
    {"Virgins"       , {"virgin"                       }, policy_standard},
    {"Indians"       , {"pajeet", "curry", "poo in loo"}, policy_standard},
    {"Indians"       , {"poos"                         }, policy_substring_risk},
    {"Blacks"        , {"nigger"                       }, policy_standard},
    {"Asians"        , {"chink"                        }, policy_standard},
    {"Web developers", {"webshit"                      }, policy_standard}
};

const search_values buzzwords = {
    {"Based"  , {"based" }, policy_standard},
    {"Seethe" , {"seeth" }, policy_standard},
    {"Cringe" , {"cringe"}, policy_standard}
};
} // namespace definitions

char remove_punctuation_helper(char c) {
    switch(c) {
    case '+':
    case '-':
    case '*':
    case '#':
        return c;
    default:
        if (ispunct(c)) {
            return ' ';
        } else {
            return c;
        }
    }
}
std::size_t count_helper(std::string_view&& post, const std::string& language) {
    constexpr std::array begin_separators = {" ", "("};
    constexpr std::array end_separators = {" ", "-", ",", ".", "?", "!", ")", "\"", "\"", "s", "fag"};
    std::size_t n = 0;
    for (auto begin_separator : begin_separators) {
        for (auto end_separator : end_separators) {
            n += toolbox::string::count(post, begin_separator + language + end_separator);
        }
    }
    if (toolbox::string::ends_with(post, " " + language) || toolbox::string::starts_with(post, language + " ")) {
        ++n;
    }
    return n;
}
std::size_t search_helper(dpt::statistics::mentions_counter& mentions, std::string post, const search_value& search_val) {
    if (!(search_val.policies & policy_no_transform)) {
        if (search_val.policies & policy_lowercase) {
            std::transform(post.begin(), post.end(), post.begin(), [](char c){ return std::tolower(c); });
        }
        if (search_val.policies & policy_no_punctuation) {
            std::transform(post.begin(), post.end(), post.begin(), remove_punctuation_helper);
            post.erase(std::unique(post.begin(), post.end(), [](char lhs, char rhs){ return (lhs == rhs) && (lhs == ' '); }), post.end());
        }
    }

    std::size_t occurences = 0;
    for (const auto& token : search_val.tokens) {
        if (search_val.policies & policy_simple_count) {
            occurences += toolbox::string::count(post, token);
        } else if (search_val.policies & policy_count_helper) {
            occurences += count_helper(post, token);
        } else if (search_val.policies & policy_exact_match) {
            if (toolbox::string::trim(post) == token) {
                occurences += 1;
            }
        } else {
            //
        }
    }

    for (const auto& token : search_val.occurs_in) {
        occurences -= toolbox::string::count(post, token);
    }

    if (occurences != 0) {
        if (search_val.policies & policy_unique) {
            mentions[search_val.key] += 1;
        } else if (search_val.policies & policy_count_all) {
            mentions[search_val.key] += occurences;
        } else {
            //
        }
    }
    return occurences;
}
} // namespace

namespace dpt {
void analyse(dpt::statistics& stats) {
    for (const auto& post : stats.posts) {
        for (const auto& search_val : definitions::programming_languages) {
            search_helper(stats.language_mentions, post.text, search_val);
            if (!post.quotes || post.quotes_op) {
                for (const auto& token : search_val.tokens) {
                    search_value single_word_val {"The word \"" + token + "\" and nothing else", {token}, policy_single_word};
                    search_helper(stats.meme_posts, post.text, single_word_val);
                }
            }
        }
        for (const auto& search_val : definitions::memes) {
            search_helper(stats.meme_posts, post.text, search_val);
        }
        for (const auto& search_val : definitions::topics) {
            search_helper(stats.topic_discussions, post.text, search_val);
        }
        for (const auto& search_val : definitions::insults) {
            search_helper(stats.insults, post.text, search_val);
        }
        for (const auto& search_val : definitions::programming_jokes) {
            search_helper(stats.programming_jokes, post.text, search_val);
        }
        for (const auto& search_val : definitions::buzzwords) {
            search_helper(stats.buzzwords, post.text, search_val);
        }
        stats.n_code_snippets += toolbox::string::count(post.text, "class=\"prettyprint\"");
    }
}
} // namespace dpt
