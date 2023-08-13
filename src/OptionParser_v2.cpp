
#include "OptionParser_v2.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

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

std::vector<std::string> defaultSuggestionsFunc(const Component &component,
                                                std::string_view input_token) {
    std::vector<std::string> suggestions;

    if (component.isParameter()) {
        return suggestions;
    }
    if (component.getName().starts_with(input_token)) {
        suggestions.push_back(component.getName());
    }

    return suggestions;
}

Component::Component(ComponentType type, std::string name,
                     std::string short_name, std::string description,
                     std::vector<Component> &&children,
                     SuggestionsFunc suggestions_func)
    : m_type(type), m_name(name), m_short_name(short_name),
      m_description(description), m_children(children),
      m_suggestions_func(suggestions_func) {}

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

std::vector<std::reference_wrapper<const Component>>
Component::getChildren() const {
    std::vector<std::reference_wrapper<const Component>> children;
    for (const auto &child : m_children) {
        children.push_back(std::ref(child));
    }
    return children;
}

std::vector<std::string>
Component::getSuggestions(std::string_view input_token) const {
    assert(m_suggestions_func);
    return m_suggestions_func(*this, input_token);
}

Component makeParameter(std::string display_name, std::string description,
                        std::vector<Component> &&children,
                        SuggestionsFunc suggestions_func) {
    return Component(ComponentType::Parameter, std::move(display_name), "",
                     std::move(description), std::move(children),
                     suggestions_func);
}

Component makeFlag(std::string name, std::string short_name,
                   std::string description, std::vector<Component> &&parameters,
                   SuggestionsFunc suggestions_func) {
    return Component(ComponentType::Flag, std::move(name),
                     std::move(short_name), std::move(description),
                     std::move(parameters), suggestions_func);
}

Component makePositionalIdentifier(std::string name, std::string description,
                                   std::vector<Component> &&children,
                                   SuggestionsFunc suggestions_func) {
    return Component(ComponentType::PositionalIdentifier, std::move(name), "",
                     std::move(description), std::move(children),
                     suggestions_func);
}

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
        if (*begin != component.getName() &&
            *begin != component.getShortName()) {
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

std::optional<ParseResult> parse(const Component &root_component,
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

std::optional<ParseResult>
parseMulti(const std::vector<Component> &root_components,
           std::string_view input_string) {

    // tokenize input_string
    std::vector<std::string_view> tokens = tokenize(input_string);

    // try to parse root_components in order, return the first successful parse
    ParseResult parse_result;
    for (const auto &root_component : root_components) {
        auto res = parseTokens(root_component, tokens.begin(), tokens.end());
        if (res.has_value()) {
            return res->first;
        }
    }

    return std::nullopt;
}

const ParseResult &getLastParseResult(const ParseResult &parse_result) {
    if (parse_result.m_children.empty()) {
        return parse_result;
    }
    return getLastParseResult(parse_result.m_children.back());
}

std::vector<std::string>
nextTokenSuggestionsComponent(const Component &component,
                              std::string_view token) {

    std::vector<std::string> suggestions;

    for (const auto &child_component_ref : component.getChildren()) {
        const Component &child_component = child_component_ref.get();

        auto child_suggestions = child_component.getSuggestions(token);
        suggestions.insert(suggestions.end(), child_suggestions.begin(),
                           child_suggestions.end());
    }

    return suggestions;
}

std::vector<std::string>
nextTokenSuggestions(const Component &root_component,
                     const std::string_view &input_string) {
    // tokenize input_string
    std::vector<std::string_view> tokens = tokenize(input_string);

    // if parse failed, suggest next token based on last token and
    // root_component
    std::reference_wrapper<const Component> component = root_component;
    std::string_view token = (tokens.empty() ? "" : tokens.back());

    // build parse tree
    auto res = parseTokens(root_component, tokens.begin(), tokens.end());
    if (res.has_value()) {
        // if parse succeeded, suggest next token based on last token and last
        // parse result
        const ParseResult &last_parse_result = getLastParseResult(res->first);
        component = *last_parse_result.m_component;

        // get last token
        if (res->second != tokens.end()) {
            //  if there are unparsed tokens, suggest next token based on last
            //  token (uncomplete)
            token = *res->second;
        } else {
            // if there are no unparsed tokens, suggest next token based on
            // empty string (the last token was complete)
            token = "";
        }
    }

    std::vector<std::string> suggestions =
        nextTokenSuggestionsComponent(root_component, token);

    return suggestions;
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
