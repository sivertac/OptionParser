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

enum class ComponentType { Parameter, Flag, PositionalIdentifier };

class Component {
public:
    Component(ComponentType type, std::string name, std::string short_name,
              std::string description, std::vector<Component> &&children = {});

    ComponentType getType() const;
    bool isParameter() const;
    bool isFlag() const;
    bool isPositionalIdentifier() const;

    const std::string &getName() const;
    const std::string &getShortName() const;
    const std::string &getDescription() const;

    std::vector<std::reference_wrapper<const Component>> getFlags() const;
    std::vector<std::reference_wrapper<const Component>>
    getPositionalIdentifiers() const;
    std::vector<std::reference_wrapper<const Component>> getParameters() const;
    std::vector<std::reference_wrapper<const Component>> getChildren() const;

private:
    ComponentType m_type;
    std::string m_name;
    std::string m_short_name;
    std::string m_description;
    std::vector<Component> m_children;
};

Component makeParameter(std::string display_name, std::string description,
                        std::vector<Component> &&children = {});

Component makeFlag(std::string name, std::string short_name,
                   std::string description,
                   std::vector<Component> &&children = {});

Component makePositionalIdentifier(std::string name, std::string description,
                                   std::vector<Component> &&children = {});

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
std::vector<std::string_view>
nextTokenSuggestions(const Component &root_component,
                     const std::string_view &input_string);

std::string serializeResult(const ParseResult &result);

} // namespace optionparser_v2

#endif // !RUNTIME_OPTION_PARSER_HEADER