#pragma once
#ifndef RUNTIME_OPTION_PARSER_HEADER
#define RUNTIME_OPTION_PARSER_HEADER

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace optionparser_v2 {

enum class ComponentType { Parameter, Flag, Command };

class Component;

/// @brief Function template to generate suggestions for a given component and
/// input_token. The suggestions are for this component, and not its children.
/// @param component The component to generate suggestions for
/// @param input_token The input token
/// @return A vector of suggestions.
using SuggestionsFunc = std::function<std::vector<std::string>(
    const Component &, std::string_view)>;

/// @brief Default suggestions function for a component.
/// @param component
/// @param input_token
/// @return Returned vector contains component.getName() if the component is not
/// of type Parameter, if not its empty.
std::vector<std::string> defaultSuggestionsFunc(const Component &component,
                                                std::string_view input_token);

class Component {
public:
    Component(ComponentType type, std::string name, std::string short_name,
              std::string description, std::vector<Component> &&children = {},
              SuggestionsFunc suggestions_func = defaultSuggestionsFunc);

    ComponentType getType() const;
    bool isParameter() const;
    bool isFlag() const;
    bool isCommand() const;

    const std::string &getName() const;
    const std::string &getShortName() const;
    const std::string &getDescription() const;

    std::vector<std::reference_wrapper<const Component>> getFlags() const;
    std::vector<std::reference_wrapper<const Component>> getCommands() const;
    std::vector<std::reference_wrapper<const Component>> getParameters() const;
    std::vector<std::reference_wrapper<const Component>> getChildren() const;

    std::vector<std::string> getSuggestions(std::string_view input_token) const;

private:
    ComponentType m_type;
    std::string m_name;
    std::string m_short_name;
    std::string m_description;
    std::vector<Component> m_children;
    SuggestionsFunc m_suggestions_func;
};

Component
makeParameter(std::string display_name, std::string description,
              std::vector<Component> &&children = {},
              SuggestionsFunc suggestions_func = defaultSuggestionsFunc);

Component makeFlag(std::string name, std::string short_name,
                   std::string description,
                   std::vector<Component> &&children = {},
                   SuggestionsFunc suggestions_func = defaultSuggestionsFunc);

Component
makeCommand(std::string name, std::string description,
            std::vector<Component> &&children = {},
            SuggestionsFunc suggestions_func = defaultSuggestionsFunc);

struct ParseResult {
    std::string_view m_value;
    const Component *m_component;
    std::vector<ParseResult> m_children;
};

/// @brief Parse the input string
/// @param root_component The root component of the parser (AST root)
/// @param input_string The input string to parse
/// @return The parse result
std::optional<ParseResult> parse(const Component &root_component,
                                 std::string_view input_string);

/// @brief Parse the input string based on multiple root components.
/// This is useful if you want to parse for multiple different types of input
/// such as diffrent commands. The root components are tried in order, and the
/// first one that succeeds is returned.
/// @param root_components
/// @param input_string
/// @return
std::optional<ParseResult>
parseMulti(const std::vector<Component> &root_components,
           std::string_view input_string);

/// @brief Get the last parse result from the parse result.
/// @param parse_result
/// @return The last parse result
const ParseResult &getLastParseResult(const ParseResult &parse_result);

/// @brief Get the next token suggestion based on the input string.
/// This is intended for tab completion, providing a list of possible tokens to
/// append to the end if the input string.
/// @param root_component The root component of the parser (AST root)
/// @param parse_result
/// @return List of possible tokens given the AST
std::vector<std::string>
nextTokenSuggestions(const Component &root_component,
                     const std::string_view &input_string);

std::vector<std::string>
nextTokenSuggestionsMulti(const std::vector<Component> &root_components,
                          const std::string_view &input_string);

std::string serializeResult(const ParseResult &result);

} // namespace optionparser_v2

#endif // !RUNTIME_OPTION_PARSER_HEADER