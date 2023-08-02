// OptionParser.hpp
// Author: Sivert Andresen Cubedo

#pragma once
#ifndef OptionParser_HEADER
#define OptionParser_HEADER

// C++
#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace OptionParser {
#define OptionParser_DEFAULT_DELIM " \t,"

/*
Split string by spaces.
*/
std::vector<std::string_view>
extractWords(std::string_view str_view,
             const std::string_view &delim = OptionParser_DEFAULT_DELIM);

/*
Remove prefix delim.
*/
std::pair<std::string_view, std::size_t>
removePrefixDelim(std::string_view str_view,
                  const std::string_view &delim = OptionParser_DEFAULT_DELIM);

/*
Get first word.
*/
std::pair<std::string_view, std::size_t>
extractFirstWord(std::string_view str_view,
                 const std::string_view &delim = OptionParser_DEFAULT_DELIM);

/*
Check if string has delim before first not delim.
*/
bool hasPrefixDelim(std::string_view str_view,
                    const std::string_view &delim = " \t");

/*
Remove first word (and delims).
*/
std::pair<std::string_view, std::size_t>
removeFirstWord(std::string_view str_view,
                const std::string_view &delim = OptionParser_DEFAULT_DELIM);

/*
Try to extract first word.
If succesful then remove word from str_view.
*/
std::optional<std::string_view> extractFirstWordDestructive(
    std::string_view &str_view,
    const std::string_view &delim = OptionParser_DEFAULT_DELIM);

/*
Search for end of list.
(first char must be start of list).
*/
std::size_t searchEndOfList(std::string_view str_view, char begin = '[',
                            char end = ']');

/*
Extract list from string.
*/
std::pair<std::string_view, std::size_t>
extractFirstList(std::string_view str_view, char begin = '[', char end = ']',
                 const std::string_view &delim = OptionParser_DEFAULT_DELIM);

/*
Parse extracted list.
*/
inline std::vector<std::string_view>
parseList(std::string_view str_view,
          const std::string_view &delim = OptionParser_DEFAULT_DELIM) {
    str_view.remove_prefix(1);
    str_view.remove_suffix(1);
    return extractWords(str_view, delim);
}

/*
Extract string.
*/
std::pair<std::string_view, std::size_t>
extractFirstString(std::string_view str_view,
                   const std::string_view &delim = OptionParser_DEFAULT_DELIM);

template <class Tup, class Func, std::size_t... Is>
constexpr void static_for_impl(Tup &t, Func &&f, std::index_sequence<Is...>) {
    (f(std::ref(std::get<Is>(t))), ...);
}

template <class... T, class Func>
constexpr void static_for(std::tuple<T...> &t, Func &&f) {
    static_for_impl(t, std::forward<Func>(f),
                    std::make_index_sequence<sizeof...(T)>{});
}

struct WordType {
    bool parseType(std::string_view &str_view, const std::string_view &delim) {
        auto pair = extractFirstWord(str_view, delim);
        if (!pair.first.empty()) {
            type = pair.first;
            str_view.remove_prefix(pair.second);
            return true;
        } else {
            return false;
        }
    }
    std::string_view type;
};
struct ListType {
    bool parseType(std::string_view &str_view, const std::string_view &delim) {
        auto pair = extractFirstList(str_view, '[', ']', delim);
        if (!pair.first.empty()) {
            type = parseList(pair.first);
            str_view.remove_prefix(pair.second);
            return true;
        } else {
            return false;
        }
    }
    std::vector<std::string_view> type;
};
struct StringType {
    bool parseType(std::string_view &str_view, const std::string_view &delim) {
        auto pair = extractFirstString(str_view, delim);
        if (!pair.first.empty()) {
            type = pair.first;
            str_view.remove_prefix(pair.second);
            return true;
        } else {
            return false;
        }
    }
    std::string_view type;
};
template <typename T> struct NumberType {
    bool parseType(std::string_view &str_view, const std::string_view &delim) {
        auto pair = extractFirstWord(str_view, delim);
        if (!pair.first.empty()) {
            // try to parse number
            auto ret = std::from_chars(
                pair.first.data(), pair.first.data() + pair.first.size(), type);
            if (ret.ec == std::errc::invalid_argument) {
                return false;
            }
            str_view.remove_prefix(pair.second);
            return true;
        } else {
            return false;
        }
    }
    T type;
};

template <class... Params> class Option;

template <class... Params> class Result {
public:
    using OptionType = Option<Params...>;
    Result() = default;

    template <std::size_t i> constexpr auto &get() const {
        return std::get<i>(m_params).type;
    }

    constexpr std::size_t getSize() const { return sizeof...(Params); }

private:
    friend OptionType;
    std::tuple<Params...> m_params;
};

template <class... Params> class Option {
public:
    using ResultType = Result<Params...>;

    Option(const std::vector<std::string_view> &identifiers)
        : m_identifiers(identifiers) {}

    Option(const std::string_view &identifier)
        : Option(std::vector<std::string_view>{identifier}) {}

    bool checkIdentifier(const std::string_view &str) const {
        auto it = std::find_if(m_identifiers.begin(), m_identifiers.end(),
                               [&](auto &id) { return str == id; });
        if (it == m_identifiers.end()) {
            return false;
        } else {
            return true;
        }
    }

    std::optional<ResultType>
    parse(std::string_view &input_view,
          const std::string_view &delim = OptionParser_DEFAULT_DELIM) const {
        if (input_view.empty())
            return std::nullopt;
        std::string_view temp_view = input_view;
        // check if identifier is correct
        auto pair = extractFirstWord(temp_view, delim);
        if (!checkIdentifier(pair.first))
            return std::nullopt;
        temp_view.remove_prefix(pair.second);

        if (sizeof...(Params) == 0) {
            input_view = temp_view;
            return ResultType();
        }

        // check and parse params
        ResultType result;
        if (parseParams(result, temp_view, delim)) {
            input_view = temp_view;
            return result;
        } else {
            return std::nullopt;
        }
    }

    constexpr std::size_t getSize() const { return sizeof...(Params); }

    constexpr auto &getIdentifiers() const { return m_identifiers; }

private:
    template <std::size_t... Is>
    bool traverseParams(ResultType &result, std::string_view &str_view,
                        const std::string_view &delim,
                        std::index_sequence<Is...>) const {
        bool failed = false;
        (
            [&](auto &type) {
                if (!failed) {
                    failed = !type.parseType(str_view, delim);
                }
            }(std::get<Is>(result.m_params)),
            ...);
        return !failed;
    }

    bool parseParams(ResultType &result, std::string_view &str_view,
                     const std::string_view &delim) const {
        return traverseParams(result, str_view, delim,
                              std::make_index_sequence<sizeof...(Params)>{});
    }

    std::vector<std::string_view> m_identifiers;
};

template <typename T1 = void, typename T2 = void, typename... Ts>
constexpr bool allSame() {
    if (std::is_void<T2>::value) {
        return true;
    } else if (std::is_same<T1, T2>::value) {
        return allSame<T2, Ts...>();
    } else {
        return false;
    }
}

template <class Param> struct OptionResult;

template <class... Params> struct OptionResult<Option<Params...>> {
    using Type = Result<Params...>;
};

template <class... Options> class Parser;

template <class... Options> class ResultSet {
public:
    using ResultTuple =
        std::tuple<std::pair<bool, typename OptionResult<Options>::Type>...>;

    ResultSet()
        : m_results(
              std::pair(false, typename OptionResult<Options>::Type())...) {}

    template <std::size_t i>
    std::optional<
        typename std::tuple_element<i, ResultTuple>::type::second_type>
    find() const {
        if (std::get<i>(m_results).first) {
            return std::get<i>(m_results).second;
        } else {
            return std::nullopt;
        }
    }

private:
    friend Parser<Options...>;
    ResultTuple m_results;
};

template <class... Options> class Parser {
public:
    using OptionTuple = std::tuple<Options...>;
    using ResultSetType = ResultSet<Options...>;

    Parser(Options... options) : m_options(options...) {}

    ResultSetType
    parse(std::string_view input_line,
          const std::string_view &delim = OptionParser_DEFAULT_DELIM) const {

        ResultSetType r;
        if (input_line.empty())
            return r;

        auto lamb_tup = std::make_tuple(
            [&](const Options &opt,
                std::pair<bool, typename OptionResult<Options>::Type> &pair)
                -> bool {
                if (!pair.first) {
                    if (auto r = opt.parse(input_line, delim)) {
                        pair.first = true;
                        pair.second = *r;
                        return true;
                    } else {
                        return false;
                    }
                } else
                    return false;
            }...);

        while (!input_line.empty()) {
            bool found = traverseOptions(
                lamb_tup, r, std::make_index_sequence<sizeof...(Options)>{});
            if (!found) {
                input_line = removeFirstWord(input_line, delim).first;
            }
        }

        return r;
    }

    constexpr const OptionTuple &getOptions() const { return m_options; }

private:
    template <class LambTup, std::size_t... Is>
    bool traverseOptions(const LambTup &lamb_tup, ResultSetType &r_set,
                         std::index_sequence<Is...>) const {
        bool found = false;
        (
            [&]() {
                if (!found) {
                    found = std::get<Is>(lamb_tup)(
                        std::get<Is>(m_options), std::get<Is>(r_set.m_results));
                }
            }(),
            ...);
        return found;
    }

    OptionTuple m_options;
};

} // namespace OptionParser

#endif // !OptionParser_HEADER
