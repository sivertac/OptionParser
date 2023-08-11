
#include "OptionParser_v2.hpp"

#include <iostream>

namespace optionparser_v2 {

template <class Container, class OutputIt, class UnaryPredicate>
OutputIt moveIfAndErase(Container &source, OutputIt d_first,
                        UnaryPredicate pred) {
    auto first = source.begin();
    auto last = source.end();
    auto pivot = std::stable_partition(first, last, pred);
    OutputIt ret = std::copy(std::make_move_iterator(first),
                             std::make_move_iterator(pivot), d_first);
    source.erase(first, pivot);
    return ret;
}

Component::Component(ComponentType type, std::string name,
                     std::string short_name, std::string description,
                     std::vector<Component> &&children) {
    m_type = type;
    m_name = std::move(name);
    m_short_name = std::move(short_name);
    m_description = std::move(description);
    m_children = std::move(children);
}

ComponentType Component::getType() const { return m_type; }
bool Component::isParameter() const {
    return m_type == ComponentType::Parameter;
}
bool Component::isFlag() const { return m_type == ComponentType::Flag; }
bool Component::isPositionalIdentifier() const {
    return m_type == ComponentType::PositionalIdentifier;
}
const std::string &Component::getName() const { return m_name; }
const std::string &Component::getShortName() const { return m_short_name; }
const std::string &Component::getDescription() const { return m_description; }

std::vector<std::reference_wrapper<const Component>>
Component::getFlags() const {
    std::vector<std::reference_wrapper<const Component>> flags;
    for (const auto &child : m_children) {
        if (child.isFlag()) {
            flags.push_back(std::ref(child));
        }
    }
    return flags;
}
std::vector<std::reference_wrapper<const Component>>
Component::getPositionalIdentifiers() const {
    std::vector<std::reference_wrapper<const Component>> positional_identifiers;
    for (const auto &child : m_children) {
        if (child.isPositionalIdentifier()) {
            positional_identifiers.push_back(std::ref(child));
        }
    }
    return positional_identifiers;
}
std::vector<std::reference_wrapper<const Component>>
Component::getParameters() const {
    std::vector<std::reference_wrapper<const Component>> parameters;
    for (const auto &child : m_children) {
        if (child.isParameter()) {
            parameters.push_back(std::ref(child));
        }
    }
    return parameters;
}

Component makeParameter(std::string display_name, std::string description,
                        std::vector<Component> &&children) {
    return Component(ComponentType::Parameter, std::move(display_name), "",
                     std::move(description), std::move(children));
}

Component makeFlag(std::string name, std::string short_name,
                   std::string description,
                   std::vector<Component> &&parameters) {
    return Component(ComponentType::Flag, std::move(name),
                     std::move(short_name), std::move(description),
                     std::move(parameters));
}

Component makePositionalIdentifier(std::string name, std::string description,
                                   std::vector<Component> &&children) {
    return Component(ComponentType::PositionalIdentifier, std::move(name), "",
                     std::move(description), std::move(children));
}

// std::optional<ParseResult> parseComplete(const Component& root_component,
// std::string_view input_string);

std::vector<std::string_view> tokenize(std::string_view input_string) {
    std::vector<std::string_view> tokens;

    size_t startPos = 0;
    size_t endPos = 0;
    bool insideQuotes = false;
    bool escaped = false;

    while (endPos < input_string.length()) {
        if (input_string[endPos] == '"' && !escaped) {
            insideQuotes = !insideQuotes;
        }

        if (input_string[endPos] == '\\' && !escaped) {
            escaped = true;
        } else {
            escaped = false;
        }

        if (!insideQuotes && input_string[endPos] == ' ' && !escaped) {
            if (endPos > startPos) {
                tokens.push_back(
                    input_string.substr(startPos, endPos - startPos));
            }
            startPos = endPos + 1;
        }

        ++endPos;
    }

    // Handle the last word if it's not followed by a space
    if (endPos > startPos) {
        tokens.push_back(input_string.substr(startPos, endPos - startPos));
    }

    // Remove quotes from tokens, if any
    for (auto &token : tokens) {
        if (token.length() >= 2 && token.front() == '"' &&
            token.back() == '"') {
            token = token.substr(1, token.length() - 2);
        }
    }

    return tokens;
}

bool isFlagName(std::string_view token, const Component &component) {
    return component.isFlag() && token == "--" + component.getName();
}

bool isFlagShortName(std::string_view token, const Component &component) {
    return component.isFlag() && token == "-" + component.getShortName();
}

std::optional<std::pair<ParseResult, std::vector<std::string_view>::iterator>>
parseTokens(const Component &component,
            std::vector<std::string_view>::iterator begin,
            std::vector<std::string_view>::iterator end);

std::optional<std::pair<std::vector<ParseResult>,
                        std::vector<std::string_view>::iterator>>
parseChildren(const Component &component,
              std::vector<std::string_view>::iterator begin,
              std::vector<std::string_view>::iterator end) {
    std::vector<ParseResult> children;

    // Presidence:
    // 1. Flags
    // 2. Positional identifiers
    // 3. Parameters

    // If we find a token that is not a valid flag, positional identifier, or
    // parameter, then stop parsing children and return what we have so far

    // for each token, check if flag or positional identifier, if not, then it's
    // a parameter
    size_t parameter_count = 0;
    while (begin != end) {
        // check if token matches any flag
        bool flag_found = false;

        for (const auto &flag : component.getFlags()) {
            auto res = parseTokens(flag, begin, end);
            if (res.has_value()) {
                children.push_back(res->first);
                begin = res->second;
                flag_found = true;
                break;
            }
        }
        if (flag_found) {
            continue;
        }

        // check if token matches any positional identifier
        bool positional_identifier_found = false;
        for (const auto &positional_identifier :
             component.getPositionalIdentifiers()) {
            auto res = parseTokens(positional_identifier, begin, end);
            if (res.has_value()) {
                children.push_back(res->first);
                begin = res->second;
                positional_identifier_found = true;
                break;
            }
        }
        if (positional_identifier_found) {
            continue;
        }

        // if we have found all parameters, then we are done
        if (parameter_count >= component.getParameters().size()) {
            break;
        }
        auto res =
            parseTokens(component.getParameters()[parameter_count], begin, end);
        if (!res.has_value()) {
            return std::nullopt;
        }
        children.push_back(res->first);
        begin = res->second;
    }

    return std::make_pair(std::move(children), begin);
}

/// @brief Parses tokens into a ParseResult
/// @param component
/// @param begin
/// @param end
/// @return pair of ParseResult and iterator to the next token if successful,
/// nullopt otherwise
std::optional<std::pair<ParseResult, std::vector<std::string_view>::iterator>>
parseTokens(const Component &component,
            std::vector<std::string_view>::iterator begin,
            std::vector<std::string_view>::iterator end) {
    if (begin == end) {
        return std::nullopt;
    }

    ParseResult parse_result;

    if (component.isParameter()) {
        parse_result = ParseResult{*begin, &component, {}};
        ++begin;
    } else if (component.isFlag()) {
        // check if token matches flag
        if (!isFlagName(*begin, component) &&
            !isFlagShortName(*begin, component)) {
            return std::nullopt;
        }
        parse_result = ParseResult{*begin, &component, {}};
        ++begin;
    } else if (component.isPositionalIdentifier()) {
        // check if token matches name
        if (*begin != component.getName()) {
            return std::nullopt;
        }
        parse_result = ParseResult{*begin, &component, {}};
        ++begin;
    }

    // parse children
    auto res = parseChildren(component, begin, end);
    if (!res.has_value()) {
        return std::nullopt;
    }
    parse_result.m_children = std::move(res->first);
    begin = res->second;

    return std::make_pair(parse_result, begin);
}

std::optional<ParseResult> parseIncomplete(const Component &root_component,
                                           std::string_view input_string) {
    // tokenize input_string
    std::vector<std::string_view> tokens = tokenize(input_string);

    // build parse tree
    auto res = parseTokens(root_component, tokens.begin(), tokens.end());
    if (!res.has_value()) {
        return std::nullopt;
    }
    return res->first;
}

std::string serializeResult(const ParseResult &result) {
    std::string output_string;

    output_string += result.m_value;

    for (const auto &child : result.m_children) {
        output_string += " ";
        output_string += serializeResult(child);
    }

    return output_string;
}

} // namespace optionparser_v2
