
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
bool Component::isCommand() const { return m_type == ComponentType::Command; }
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
Component::getCommands() const {
    std::vector<std::reference_wrapper<const Component>> commands;
    for (const auto &child : m_children) {
        if (child.isCommand()) {
            commands.push_back(std::ref(child));
        }
    }
    return commands;
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

std::vector<Component> &Component::getChildrenMutable() { return m_children; }

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

Component makeCommand(std::string name, std::string description,
                      std::vector<Component> &&children,
                      SuggestionsFunc suggestions_func) {
    return Component(ComponentType::Command, std::move(name), "",
                     std::move(description), std::move(children),
                     suggestions_func);
}

ParseContext::ParseContext(
    const std::vector<std::reference_wrapper<const Component>> root_components)
    : m_root_components(root_components) {}

ParseContext::ParseContext(const std::vector<Component> &root_components) {
    for (const auto &root_component : root_components) {
        m_root_components.push_back(std::ref(root_component));
    }
}

ParseContext::ParseContext(const Component &root_component) {
    m_root_components.push_back(std::ref(root_component));
}

std::optional<ParseResult> parseResultFromToken(const Component &component,
                                                std::string_view token) {
    ParseResult parse_result;
    if (component.isParameter()) {
        parse_result = ParseResult{std::string(token), &component, {}};
    } else if (component.isFlag()) {
        // check if token matches flag
        if (token != component.getName() && token != component.getShortName()) {
            return std::nullopt;
        }
        parse_result = ParseResult{std::string(token), &component, {}};
    } else if (component.isCommand()) {
        // check if token matches name
        if (token != component.getName()) {
            return std::nullopt;
        }
        parse_result = ParseResult{std::string(token), &component, {}};
    } else {
        assert(false);
    }
    return parse_result;
}

size_t countParametersInParseResult(const ParseResult &parse_result) {
    size_t count = 0;
    // count number of parameters present in children, non recursive
    for (const auto &child : parse_result.m_children) {
        if (child.m_component->isParameter()) {
            ++count;
        }
    }
    return count;
}

bool ParseContext::parseToken(std::string_view token) {
    // if first token
    if (!m_root_parse_result.has_value()) {
        // try to find a matching root component
        for (const auto &component : m_root_components) {
            auto parse_result = parseResultFromToken(component, token);
            if (parse_result) {
                m_root_parse_result = std::move(parse_result);
                m_parse_result_stack.push(&m_root_parse_result.value());
                return true;
            }
        }
        return false;
    }

    // if not first token

    // try to find a matching child component
    // Presidence:
    // 1. Flags
    // 2. Commands
    // 3. Parameters
    // If we find a token that is not a valid flag, command, or
    // parameter, then stop parsing children and return what we have so far
    while (!m_parse_result_stack.empty()) {
        ParseResult &parse_result = *m_parse_result_stack.top();
        const Component &component = *parse_result.m_component;

        if (component.getChildren().empty()) {
            // if component has no children, then there is nothing to parse
            return false;
        }

        std::optional<ParseResult> child_parse_result;
        for (const Component &flag : component.getFlags()) {
            child_parse_result = parseResultFromToken(flag, token);
            if (child_parse_result.has_value()) {
                break;
            }
        }

        if (!child_parse_result.has_value()) {
            for (const Component &command : component.getCommands()) {
                child_parse_result = parseResultFromToken(command, token);
                if (child_parse_result.has_value()) {
                    break;
                }
            }
        }

        if (!child_parse_result.has_value()) {
            // if we have found all parameters, then we are done
            size_t parameter_count = countParametersInParseResult(parse_result);
            if (parameter_count < component.getParameters().size()) {
                child_parse_result = parseResultFromToken(
                    component.getParameters()[parameter_count], token);
            }
        }

        if (child_parse_result.has_value()) {
            parse_result.m_children.push_back(
                std::move(child_parse_result.value()));
            // if child has children, then push it onto the stack
            if (!parse_result.m_children.back()
                     .m_component->getChildren()
                     .empty()) {
                m_parse_result_stack.push(&parse_result.m_children.back());
            }
            return true;
        }

        // if we didn't match any child component, and all parameters are
        // consumed, pop the stack
        m_parse_result_stack.pop();
    }
    return false;
}

std::vector<std::string>
nextTokenSuggestionsParseResult(const ParseResult &parse_result,
                                std::string_view token) {

    std::vector<std::string> suggestions;

    size_t result_parameter_count = countParametersInParseResult(parse_result);
    size_t component_parameter_count =
        parse_result.m_component->getParameters().size();

    for (const auto &child_component_ref :
         parse_result.m_component->getChildren()) {
        const Component &child_component = child_component_ref.get();

        // if we have found all parameters, don't suggest any more parameters
        if (result_parameter_count >= component_parameter_count &&
            child_component.isParameter()) {
            continue;
        }

        auto child_suggestions = child_component.getSuggestions(token);
        suggestions.insert(suggestions.end(), child_suggestions.begin(),
                           child_suggestions.end());
    }

    return suggestions;
}

std::vector<std::string>
ParseContext::getNextSuggestions(std::string_view token) const {
    if (m_parse_result_stack.empty()) {
        if (m_root_parse_result.has_value()) {
            return {};
        }
        // if we have no parse result, then suggest next token based on root
        // components
        std::vector<std::string> suggestions;
        for (const auto &root_component : m_root_components) {
            auto root_component_suggestions =
                root_component.get().getSuggestions(token);
            suggestions.insert(suggestions.end(),
                               root_component_suggestions.begin(),
                               root_component_suggestions.end());
        }
        return suggestions;
    }
    const ParseResult &parse_result = *m_parse_result_stack.top();
    return nextTokenSuggestionsParseResult(parse_result, token);
}

const std::optional<ParseResult> &ParseContext::getRootParseResult() const {
    return m_root_parse_result;
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

std::optional<ParseResult> parseMultiImpl(
    const std::vector<std::reference_wrapper<const Component>> &root_components,
    std::string_view input_string) {

    // tokenize input_string
    std::vector<std::string_view> tokens = tokenize(input_string);

    ParseContext parse_context(root_components);
    for (const auto &token : tokens) {
        if (!parse_context.parseToken(token)) {
            return std::nullopt;
        }
    }
    return parse_context.getRootParseResult();
}

std::optional<ParseResult>
parseMulti(const std::vector<Component> &root_components,
           std::string_view input_string) {
    std::vector<std::reference_wrapper<const Component>> root_component_refs;
    for (const auto &root_component : root_components) {
        root_component_refs.push_back(std::ref(root_component));
    }
    return parseMultiImpl(root_component_refs, input_string);
}

std::optional<ParseResult> parse(const Component &root_component,
                                 std::string_view input_string) {
    return parseMultiImpl({std::ref(root_component)}, input_string);
}

const ParseResult &getLastParseResult(const ParseResult &parse_result) {
    if (parse_result.m_children.empty()) {
        return parse_result;
    }
    return getLastParseResult(parse_result.m_children.back());
}

std::vector<std::string> nextTokenSuggestionsMultiImpl(
    const std::vector<std::reference_wrapper<const Component>> &root_components,
    const std::string_view &input_string) {

    // tokenize input_string
    std::vector<std::string_view> tokens = tokenize(input_string);

    ParseContext parse_context(root_components);
    auto it = std::begin(tokens);
    for (; it != std::end(tokens); ++it) {
        // if the last token maches the end of the input_string, don't parse it

        if (it == std::end(tokens) - 1 && input_string.ends_with(*it)) {
            break;
        }

        if (!parse_context.parseToken(*it)) {
            break;
        }
    }
    if (it != std::end(tokens)) {
        // if parse failed, suggest next token based on last token and
        // root_component
        return parse_context.getNextSuggestions(*it);
    }
    return parse_context.getNextSuggestions();
}

std::vector<std::string>
nextTokenSuggestionsMulti(const std::vector<Component> &root_components,
                          const std::string_view &input_string) {

    std::vector<std::reference_wrapper<const Component>> root_component_refs;
    for (const auto &root_component : root_components) {
        root_component_refs.push_back(std::ref(root_component));
    }
    return nextTokenSuggestionsMultiImpl(root_component_refs, input_string);
}

std::vector<std::string>
nextTokenSuggestions(const Component &root_component,
                     const std::string_view &input_string) {
    return nextTokenSuggestionsMultiImpl({std::ref(root_component)},
                                         input_string);
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
