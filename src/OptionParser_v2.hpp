#pragma once
#ifndef RUNTIME_OPTION_PARSER_HEADER
#define RUNTIME_OPTION_PARSER_HEADER

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <stack>
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
              SuggestionsFunc suggestions_func = defaultSuggestionsFunc,
              bool required = false);

    ComponentType getType() const;
    bool isParameter() const;
    bool isFlag() const;
    bool isCommand() const;

    bool isRequired() const;

    const std::string &getName() const;
    const std::string &getShortName() const;
    const std::string &getDescription() const;

    std::vector<std::reference_wrapper<const Component>> getFlags() const;
    std::vector<std::reference_wrapper<const Component>> getCommands() const;
    std::vector<std::reference_wrapper<const Component>> getParameters() const;
    std::vector<std::reference_wrapper<const Component>> getChildren() const;

    std::vector<Component> &getChildrenMutable();

    std::vector<std::string> getSuggestions(std::string_view input_token) const;

private:
    ComponentType m_type;
    std::string m_name;
    std::string m_short_name;
    std::string m_description;
    std::vector<Component> m_children;
    SuggestionsFunc m_suggestions_func;
    bool m_required;
};

Component
makeParameter(std::string display_name, std::string description,
              std::vector<Component> &&children = {},
              SuggestionsFunc suggestions_func = defaultSuggestionsFunc,
              bool required = false);

Component makeRequiredParameter(
    std::string display_name, std::string description,
    std::vector<Component> &&children = {},
    SuggestionsFunc suggestions_func = defaultSuggestionsFunc);

Component makeFlag(std::string name, std::string short_name,
                   std::string description,
                   std::vector<Component> &&children = {},
                   SuggestionsFunc suggestions_func = defaultSuggestionsFunc,
                   bool required = false);

Component
makeRequiredFlag(std::string name, std::string short_name,
                 std::string description,
                 std::vector<Component> &&children = {},
                 SuggestionsFunc suggestions_func = defaultSuggestionsFunc);

Component makeCommand(std::string name, std::string description,
                      std::vector<Component> &&children = {},
                      SuggestionsFunc suggestions_func = defaultSuggestionsFunc,
                      bool required = false);

Component
makeRequiredCommand(std::string name, std::string description,
                    std::vector<Component> &&children = {},
                    SuggestionsFunc suggestions_func = defaultSuggestionsFunc);

struct ParseResult {
    std::string m_value;
    const Component *m_component;
    std::vector<ParseResult> m_children;
};

/// @brief Parse context for parsing a string.
class ParseContext {
public:
    ParseContext(const std::vector<std::reference_wrapper<const Component>>
                     root_components);
    ParseContext(const std::vector<Component> &root_components);
    ParseContext(const Component &root_component);

    /// @brief Add and parse token in context, and update the parse result.
    /// @param token
    /// @return True if the token was successfully parsed, false otherwise.
    bool parseToken(std::string_view token);

    /// @brief Get the suggestions for the next token.
    /// @return
    std::vector<std::string>
    getNextSuggestions(std::string_view token = "") const;

    /// @brief Get the root parse result.
    /// @return
    const std::optional<ParseResult> &getRootParseResult() const;

    /// @brief Check if the parse is complete.
    /// A parse is complete if all ParseResults does not have any pending
    /// required children.
    /// @return
    bool isComplete() const;

private:
    std::vector<std::reference_wrapper<const Component>> m_root_components;
    std::optional<ParseResult> m_root_parse_result = std::nullopt;
    std::stack<ParseResult *> m_parse_result_stack;
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

/// @brief Generate a usage string for the given component.
/// @param root_component
/// @return
std::string generateUsageString(const Component &root_component);

/// @brief Generate a help string for the given component.
/// @param root_component
/// @return
std::string generateHelpString(const Component &root_component,
                               int margin = 40);

} // namespace optionparser_v2

#endif // !RUNTIME_OPTION_PARSER_HEADER