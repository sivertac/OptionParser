
#include "OptionParser.hpp"

namespace OptionParser {

std::vector<std::string_view> extractWords(std::string_view str_view,
                                           const std::string_view &delim) {
    std::vector<std::string_view> word_vec;
    while (!str_view.empty()) {
        std::size_t delim_pos = str_view.find_first_of(delim);
        std::size_t word_pos = str_view.find_first_not_of(delim);
        if (delim_pos == str_view.npos) {
            word_vec.push_back(str_view);
            break;
        } else if (word_pos == str_view.npos) {
            break;
        } else if (delim_pos > word_pos) {
            word_vec.push_back(str_view.substr(0, delim_pos));
            str_view.remove_prefix(delim_pos);
        } else {
            str_view.remove_prefix(word_pos);
        }
    }
    return word_vec;
}

std::pair<std::string_view, std::size_t>
removePrefixDelim(std::string_view str_view, const std::string_view &delim) {
    auto delim_pos = str_view.find_first_of(delim);
    if (delim_pos == str_view.npos)
        return std::make_pair(str_view, 0);
    auto word_pos = str_view.find_first_not_of(delim);
    if (word_pos == str_view.npos) {
        std::size_t s = str_view.size();
        str_view.remove_prefix(str_view.size());
        return std::make_pair(str_view, s);
    }
    if (word_pos < delim_pos) {
        return std::make_pair(str_view, 0);
    } else {
        str_view.remove_prefix(word_pos);
        return std::make_pair(str_view, word_pos);
    }
}

std::pair<std::string_view, std::size_t>
extractFirstWord(std::string_view str_view, const std::string_view &delim) {
    auto pair = removePrefixDelim(str_view, delim);
    auto delim_pos = pair.first.find_first_of(delim);
    if (delim_pos == pair.first.npos)
        return std::make_pair(pair.first, pair.second + pair.first.size());
    auto word_pos = pair.first.find_first_not_of(delim);
    if (word_pos == pair.first.npos) {
        str_view.remove_prefix(pair.first.size());
        return std::make_pair(pair.first, pair.second);
    }
    return std::make_pair(pair.first.substr(0, delim_pos),
                          pair.second + delim_pos);
}

bool hasPrefixDelim(std::string_view str_view, const std::string_view &delim) {
    auto delim_pos = str_view.find_first_of(delim);
    auto word_pos = str_view.find_first_not_of(delim);
    if (delim_pos == str_view.npos && word_pos == str_view.npos) {
        return false;
    } else if (delim_pos == str_view.npos) {
        return false;
    } else if (word_pos == str_view.npos) {
        return true;
    } else if (delim_pos > word_pos) {
        return false;
    } else /* if (delim_pos < word_pos) */ {
        return true;
    }
}

std::pair<std::string_view, std::size_t>
removeFirstWord(std::string_view str_view, const std::string_view &delim) {
    std::size_t s = 0;
    auto pair = removePrefixDelim(str_view, delim);
    s += pair.second;
    auto first_word = extractFirstWord(pair.first, delim);
    pair.first.remove_prefix(first_word.first.size());
    s += first_word.first.size();
    pair = removePrefixDelim(pair.first, delim);
    s += pair.second;
    return std::make_pair(pair.first, s);
}

std::optional<std::string_view>
extractFirstWordDestructive(std::string_view &str_view,
                            const std::string_view &delim) {
    auto pair1 = extractFirstWord(str_view, delim);
    if (pair1.first.empty()) {
        return std::nullopt;
    } else {
        auto pair2 = removeFirstWord(str_view, delim);
        str_view = pair2.first;
        return pair1.first;
    }
}

std::size_t searchEndOfList(std::string_view str_view, char begin, char end) {
    assert(begin != end);

    if (str_view.empty())
        return str_view.npos;

    str_view.remove_prefix(1); // remove first begin
    std::size_t count = 1;

    std::size_t total_pos = 0;

    while (count > 0) {
        auto end_pos = str_view.find(end);
        if (end_pos == str_view.npos)
            return str_view.npos;
        auto begin_pos = str_view.find(begin);
        if (begin_pos < end_pos) {
            str_view.remove_prefix(begin_pos + 1);
            total_pos += begin_pos + 1;
            ++count;
        } else {
            str_view.remove_prefix(end_pos + 1);
            total_pos += end_pos + 1;
            --count;
        }
    }
    return total_pos;
}

std::pair<std::string_view, std::size_t>
extractFirstList(std::string_view str_view, char begin, char end,
                 const std::string_view &delim) {
    assert(begin != end);
    auto pair = removePrefixDelim(str_view, delim);
    auto end_pos = searchEndOfList(pair.first, begin, end);
    if (end_pos == pair.first.npos) {
        return std::make_pair(pair.first.substr(0, 0), pair.second);
    } else {
        return std::make_pair(pair.first.substr(0, end_pos + 1),
                              pair.second + end_pos + 1);
    }
}

std::pair<std::string_view, std::size_t>
extractFirstString(std::string_view str_view, const std::string_view &delim) {
    auto pair = removePrefixDelim(str_view, delim);

    if (pair.first.empty() || pair.first.front() != '"') {
        return std::make_pair(pair.first.substr(0, 0), pair.second);
    }

    pair.first.remove_prefix(1);
    auto end = pair.first.find_first_of('"');
    if (end == pair.first.npos) {
        return std::make_pair(pair.first.substr(0, 0), pair.second + 1);
    }
    return std::make_pair(pair.first.substr(0, end), pair.second + 1 + end + 1);
}

} // namespace OptionParser
