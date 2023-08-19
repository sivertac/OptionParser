
#include <OptionParser_v2.cpp>
#include <OptionParser_v2.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(tokenize, EmptyString) {
    std::string_view input_string = "";
    std::vector<std::string_view> tokens =
        optionparser_v2::tokenize(input_string);
    EXPECT_EQ(tokens.size(), 0);
}

TEST(tokenize, OneWord) {
    std::string_view input_string = "one";
    std::vector<std::string_view> tokens =
        optionparser_v2::tokenize(input_string);
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0], "one");
}

TEST(tokenize, ManyWords) {
    std::string_view input_string = "one two three";
    std::vector<std::string_view> tokens =
        optionparser_v2::tokenize(input_string);
    EXPECT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "one");
    EXPECT_EQ(tokens[1], "two");
    EXPECT_EQ(tokens[2], "three");
}

TEST(tokenize, ManySpaces) {
    std::string_view input_string = "   one  two   three";
    std::vector<std::string_view> tokens =
        optionparser_v2::tokenize(input_string);
    EXPECT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "one");
    EXPECT_EQ(tokens[1], "two");
    EXPECT_EQ(tokens[2], "three");
}

TEST(tokenize, QuotedString) {
    std::string_view input_string = "one \"two three\"";
    std::vector<std::string_view> tokens =
        optionparser_v2::tokenize(input_string);
    EXPECT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "one");
    EXPECT_EQ(tokens[1], "two three");
}

TEST(tokenize, QuotedStringWithSpaces) {
    std::string_view input_string = "one \"two three\" four";
    std::vector<std::string_view> tokens =
        optionparser_v2::tokenize(input_string);
    EXPECT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "one");
    EXPECT_EQ(tokens[1], "two three");
    EXPECT_EQ(tokens[2], "four");
}

TEST(tokenize, QuotedStringWithEscapedQuote) {
    std::string_view input_string = "one \"two \\\"three\" four";
    std::vector<std::string_view> tokens =
        optionparser_v2::tokenize(input_string);
    EXPECT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "one");
    EXPECT_EQ(tokens[1], "two \\\"three");
    EXPECT_EQ(tokens[2], "four");
}

TEST(ParseContext_parseToken, RecursiveHeterogeneousTree) {
    using namespace optionparser_v2;
    int depth = 100;
    Component root_component = makeCommand("root", "root");
    Component *current_component = &root_component;
    for (int i = 0; i < depth; ++i) {
        Component child_component = makeCommand(std::to_string(i), "child");
        current_component->getChildrenMutable().push_back(
            std::move(child_component));
        current_component = &current_component->getChildrenMutable().back();
    }

    ParseContext context(root_component);
    EXPECT_TRUE(context.parseToken("root"));
    for (int i = 0; i < depth; ++i) {
        EXPECT_TRUE(context.parseToken(std::to_string(i)));
    }

    EXPECT_FALSE(context.parseToken("notfound"));

    EXPECT_TRUE(context.getRootParseResult().has_value());
    const ParseResult &root_parse_result = context.getRootParseResult().value();
    EXPECT_EQ(root_parse_result.m_value, "root");
    ParseResult const *current_parse_result = &root_parse_result.m_children[0];
    for (int i = 0; i < depth; ++i) {
        EXPECT_EQ(current_parse_result->m_value, std::to_string(i));
        current_parse_result = &current_parse_result->m_children[0];
    }
}

TEST(ParseContext_parseToken, RequiredFlagComplete) {
    using namespace optionparser_v2;
    auto root_command = makeCommand(
        "git", "git",
        {makeRequiredFlag("--help", "-h", "Print help message", {})});

    ParseContext context(root_command);
    EXPECT_TRUE(context.parseToken("git"));
    EXPECT_TRUE(context.parseToken("--help"));
    EXPECT_TRUE(context.getRootParseResult().has_value());
    EXPECT_EQ(context.getRootParseResult().value().m_value, "git");
    EXPECT_EQ(context.getRootParseResult().value().m_children.size(), 1);
    EXPECT_EQ(context.getRootParseResult().value().m_children[0].m_value,
              "--help");
    EXPECT_TRUE(context.isComplete());
}

TEST(ParseContext_parseToken, RequiredFlagIncomplete) {
    using namespace optionparser_v2;
    auto root_command = makeCommand(
        "git", "git",
        {makeRequiredFlag("--help", "-h", "Print help message", {})});

    ParseContext context(root_command);
    EXPECT_TRUE(context.parseToken("git"));
    EXPECT_FALSE(context.isComplete());
    EXPECT_FALSE(context.parseToken("--wrong"));
    EXPECT_FALSE(context.isComplete());
}

TEST(ParseContext_parseToken, RequiredParameterComplete) {
    using namespace optionparser_v2;
    auto root_command =
        makeCommand("git", "git",
                    {makeRequiredParameter("param1", "param 1", {}),
                     makeParameter("param2", "param 2", {})});

    ParseContext context(root_command);
    EXPECT_TRUE(context.parseToken("git"));
    EXPECT_TRUE(context.parseToken("param1"));
    EXPECT_TRUE(context.getRootParseResult().has_value());
    EXPECT_EQ(context.getRootParseResult().value().m_value, "git");
    EXPECT_EQ(context.getRootParseResult().value().m_children.size(), 1);
    EXPECT_EQ(context.getRootParseResult().value().m_children[0].m_value,
              "param1");
    EXPECT_TRUE(context.isComplete());
}

TEST(ParseContext_parseToken, RequiredParameterIncomplete) {
    using namespace optionparser_v2;
    auto root_command =
        makeCommand("git", "git",
                    {makeRequiredParameter("param1", "param 1", {}),
                     makeParameter("param2", "param 2", {})});

    ParseContext context(root_command);
    EXPECT_TRUE(context.parseToken("git"));
    EXPECT_FALSE(context.isComplete());
    EXPECT_TRUE(context.parseToken("param3"));
    EXPECT_TRUE(context.isComplete());
    EXPECT_TRUE(context.parseToken("param2"));
    EXPECT_TRUE(context.isComplete());
}

TEST(ParseContext_parseToken, NotrequiredCommandComplete) {
    using namespace optionparser_v2;
    auto root_command = makeCommand("git", "git",
                                    {makeCommand("command1", "command 1", {}),
                                     makeCommand("command2", "command 2", {})});

    ParseContext context(root_command);
    EXPECT_FALSE(context.isComplete());
    EXPECT_TRUE(context.parseToken("git"));
    EXPECT_TRUE(context.isComplete());
}

TEST(ParseContext_parseToken, MultipleRequiredCommandRootLevel) {
    using namespace optionparser_v2;
    std::vector<Component> root_commands = {
        makeRequiredCommand("one", "one", {}),
        makeRequiredCommand("two", "two", {}),
    };

    ParseContext context(root_commands);
    EXPECT_FALSE(context.isComplete());
    EXPECT_TRUE(context.parseToken("one"));
    EXPECT_TRUE(context.isComplete());
}

TEST(ParseContext_getNextSuggestions, OnlySuggestNParameters) {
    using namespace optionparser_v2;
    auto root_command = makeCommand(
        "git", "git",
        {makeParameter(
            "one", "one", {}, [](const Component &, std::string_view) {
                return std::vector<std::string>{"one", "two", "three"};
            })});

    ParseContext context(root_command);
    EXPECT_TRUE(context.parseToken("git"));
    EXPECT_EQ(context.getNextSuggestions().size(), 3);
    EXPECT_TRUE(context.parseToken("one"));
    EXPECT_EQ(context.getNextSuggestions().size(), 0);
    EXPECT_FALSE(context.parseToken("two"));
}

TEST(parse, EmptyString) {
    using namespace optionparser_v2;
    auto flag = makeFlag("--help", "-h", "Print help message", {});

    std::string_view input_string = "";
    std::optional<ParseResult> result =
        optionparser_v2::parse(flag, input_string);
    EXPECT_FALSE(result.has_value());
}

TEST(parse, ParameterOneWord) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("url", "URL of repository");

    std::string_view input_string = "randomurl";
    std::optional<ParseResult> result =
        optionparser_v2::parse(parameter, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "randomurl");
    EXPECT_TRUE(result->m_component->isParameter());
}

TEST(parse, FlagOneWord) {
    using namespace optionparser_v2;
    auto flag = makeFlag("--help", "-h", "Print help message", {});

    std::string_view input_string = "--help";
    std::optional<ParseResult> result =
        optionparser_v2::parse(flag, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "--help");
    EXPECT_TRUE(result->m_component->isFlag());
}

TEST(parse, CommandOneWord) {
    using namespace optionparser_v2;
    auto command = makeCommand("clone", "Clone a repository", {});

    std::string_view input_string = "clone";
    std::optional<ParseResult> result =
        optionparser_v2::parse(command, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "clone");
    EXPECT_TRUE(result->m_component->isCommand());
}

TEST(parse, CommandContainingParameterAndFlag) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("url", "URL of repository");
    auto flag = makeFlag("--help", "-h", "Print help message", {});
    std::vector<Component> children;
    children.push_back(std::move(parameter));
    children.push_back(std::move(flag));
    auto command =
        makeCommand("clone", "Clone a repository", std::move(children));

    std::string_view input_string = "clone --help yo";
    std::optional<ParseResult> result =
        optionparser_v2::parse(command, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "clone");
    EXPECT_EQ(result->m_component, &command);
    EXPECT_EQ(result->m_children.size(), 2);
    EXPECT_EQ(result->m_children[0].m_value, "--help");
    EXPECT_TRUE(result->m_children[0].m_component->isFlag());
    EXPECT_EQ(result->m_children[1].m_value, "yo");
    EXPECT_TRUE(result->m_children[1].m_component->isParameter());
}

TEST(parse, CommandContainingParameterAndFlagAndCommand) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("url", "URL of repository");
    auto flag = makeFlag("--help", "-h", "Print help message", {});
    auto command = makeCommand("clone", "Clone a repository", {});
    std::vector<Component> children;
    children.push_back(std::move(parameter));
    children.push_back(std::move(flag));
    children.push_back(std::move(command));
    auto command2 =
        makeCommand("pull", "Pull a repository", std::move(children));

    std::string_view input_string = "pull clone --help yo";
    std::optional<ParseResult> result =
        optionparser_v2::parse(command2, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "pull");
    EXPECT_EQ(result->m_component, &command2);
    EXPECT_EQ(result->m_children.size(), 3);
    EXPECT_EQ(result->m_children[0].m_value, "clone");
    EXPECT_TRUE(result->m_children[0].m_component->isCommand());
    EXPECT_EQ(result->m_children[1].m_value, "--help");
    EXPECT_TRUE(result->m_children[1].m_component->isFlag());
    EXPECT_EQ(result->m_children[2].m_value, "yo");
    EXPECT_TRUE(result->m_children[2].m_component->isParameter());
}

TEST(parseMulti, TwoCommands) {
    using namespace optionparser_v2;
    auto command1 = makeCommand("clone", "Clone a repository", {});
    auto command2 = makeCommand("pull", "Pull a repository", {});
    std::vector<Component> commands;
    commands.push_back(std::move(command1));
    commands.push_back(std::move(command2));

    std::string_view input_string = "clone";
    std::optional<ParseResult> result =
        optionparser_v2::parseMulti(commands, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "clone");
    EXPECT_EQ(result->m_component, &commands[0]);

    input_string = "pull";
    result = optionparser_v2::parseMulti(commands, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "pull");
    EXPECT_EQ(result->m_component, &commands[1]);

    input_string = "none";
    result = optionparser_v2::parseMulti(commands, input_string);
    EXPECT_FALSE(result.has_value());
}

TEST(parse, FlagEmptyShortName) {
    using namespace optionparser_v2;
    auto flag = makeFlag("--help", "", "Print help message", {});

    std::string_view input_string = "--help";
    std::optional<ParseResult> result =
        optionparser_v2::parse(flag, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "--help");
    EXPECT_TRUE(result->m_component->isFlag());

    input_string = "   ";
    result = optionparser_v2::parse(flag, input_string);
    EXPECT_FALSE(result.has_value());
}

TEST(parse, FlagEmptyName) {
    using namespace optionparser_v2;
    auto flag = makeFlag("", "-h", "Print help message", {});

    std::string_view input_string = "-h";
    std::optional<ParseResult> result =
        optionparser_v2::parse(flag, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "-h");
    EXPECT_TRUE(result->m_component->isFlag());

    input_string = "   ";
    result = optionparser_v2::parse(flag, input_string);
    EXPECT_FALSE(result.has_value());
}

TEST(serializeResult, TwoParameters) {
    using namespace optionparser_v2;
    auto parameter1 =
        makeParameter("one", "one", {makeParameter("two", "two")});

    std::string_view input_string = "one two";
    std::optional<ParseResult> result =
        optionparser_v2::parse(parameter1, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "one");
    EXPECT_TRUE(result->m_component->isParameter());

    std::string serialized = optionparser_v2::serializeResult(*result);
    EXPECT_EQ(serialized, input_string);
}

TEST(serializeResult, FlagWithParameter) {
    using namespace optionparser_v2;
    auto parameter1 =
        makeParameter("one", "one", {makeParameter("two", "two")});
    auto flag = makeFlag("--help", "-h", "Print help message", {parameter1});

    std::string_view input_string = "--help one two";
    std::optional<ParseResult> result =
        optionparser_v2::parse(flag, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "--help");
    EXPECT_TRUE(result->m_component->isFlag());

    std::string serialized = optionparser_v2::serializeResult(*result);
    EXPECT_EQ(serialized, input_string);
}

TEST(serializeResult, CommandWithFlagAndParameter) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("one", "one");
    auto flag = makeFlag("--help", "-h", "Print help message");
    auto command =
        makeCommand("clone", "Clone a repository", {flag, parameter});

    std::string_view input_string = "clone one --help";
    std::optional<ParseResult> result =
        optionparser_v2::parse(command, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "clone");
    EXPECT_TRUE(result->m_component->isCommand());

    std::string serialized = optionparser_v2::serializeResult(*result);
    EXPECT_EQ(serialized, input_string);
}

TEST(nextTokenSuggestions, TwoCommands) {
    using namespace optionparser_v2;
    auto command_root = makeCommand(
        "git", "git",
        {makeCommand("clone", "clone"), makeCommand("pull", "pull")});

    std::string_view input_string = "git ";

    std::vector<std::string> suggestions =
        optionparser_v2::nextTokenSuggestions(command_root, input_string);
    EXPECT_EQ(suggestions.size(), 2);
    EXPECT_THAT(suggestions, ::testing::Contains("clone"));
    EXPECT_THAT(suggestions, ::testing::Contains("pull"));
}

TEST(nextTokenSuggestions, TwoCommandsPrefix) {
    using namespace optionparser_v2;
    auto command_root = makeCommand(
        "git", "git",
        {makeCommand("clone", "clone"), makeCommand("pull", "pull")});

    std::string_view input_string = "git p";

    std::vector<std::string> suggestions =
        optionparser_v2::nextTokenSuggestions(command_root, input_string);
    EXPECT_EQ(suggestions.size(), 1);
    EXPECT_THAT(suggestions, ::testing::Contains("pull"));
}

TEST(nextTokenSuggestions, CommandsNotFound) {
    using namespace optionparser_v2;
    auto command_root = makeCommand(
        "git", "git",
        {makeCommand("clone", "clone"), makeCommand("pull", "pull")});

    std::string_view input_string = "gsd";

    std::vector<std::string> suggestions =
        optionparser_v2::nextTokenSuggestions(command_root, input_string);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST(nextTokenSuggestions, Parameter) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("url", "URL of repository");
    auto command = makeCommand("clone", "Clone a repository", {parameter});

    std::string_view input_string = "clone ";

    std::vector<std::string> suggestions =
        optionparser_v2::nextTokenSuggestions(command, input_string);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST(nextTokenSuggestions, Flag) {
    using namespace optionparser_v2;
    auto flag = makeFlag("--help", "-h", "Print help message");
    auto command = makeCommand("clone", "Clone a repository", {flag});

    std::string_view input_string = "clone ";

    std::vector<std::string> suggestions =
        optionparser_v2::nextTokenSuggestions(command, input_string);
    EXPECT_EQ(suggestions.size(), 1);
    EXPECT_THAT(suggestions, ::testing::Contains("--help"));
}

TEST(nextTokenSuggestions, ParameterCustomSuggestions) {
    using namespace optionparser_v2;
    auto parameter = makeParameter(
        "url", "URL of repository", {},
        [](const Component &, std::string_view) {
            return std::vector<std::string>{"one", "two", "three"};
        });
    auto command = makeCommand("clone", "Clone a repository", {parameter});

    std::string_view input_string = "clone ";

    std::vector<std::string> suggestions =
        optionparser_v2::nextTokenSuggestions(command, input_string);
    EXPECT_EQ(suggestions.size(), 3);
    EXPECT_THAT(suggestions, ::testing::Contains("one"));
    EXPECT_THAT(suggestions, ::testing::Contains("two"));
    EXPECT_THAT(suggestions, ::testing::Contains("three"));
}

TEST(nextTokenSuggestions, SpaceVsNoSpaceInputStringEnd) {
    using namespace optionparser_v2;
    auto command = makeCommand("clone", "Clone a repository", {});

    std::string_view input_string = "clone";

    std::vector<std::string> suggestions =
        optionparser_v2::nextTokenSuggestions(command, input_string);
    EXPECT_EQ(suggestions.size(), 1);
    EXPECT_THAT(suggestions, ::testing::Contains("clone"));

    input_string = "clone ";

    suggestions = optionparser_v2::nextTokenSuggestions(command, input_string);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST(nextTokenSuggestionsMulti, MultipleCommandRoots) {
    using namespace optionparser_v2;
    auto command_root1 = makeCommand(
        "one", "one",
        {makeCommand("clone", "clone"), makeCommand("pull", "pull")});
    auto command_root2 = makeCommand(
        "two", "two",
        {makeCommand("commit", "commit"), makeCommand("push", "push")});

    std::string_view input_string;
    std::vector<std::string> suggestions;

    input_string = "";

    suggestions = optionparser_v2::nextTokenSuggestionsMulti(
        {command_root1, command_root2}, input_string);

    EXPECT_EQ(suggestions.size(), 2);
    EXPECT_THAT(suggestions, ::testing::Contains("one"));
    EXPECT_THAT(suggestions, ::testing::Contains("two"));

    input_string = "o";

    suggestions = optionparser_v2::nextTokenSuggestionsMulti(
        {command_root1, command_root2}, input_string);

    EXPECT_EQ(suggestions.size(), 1);
    EXPECT_THAT(suggestions, ::testing::Contains("one"));

    input_string = "two";

    suggestions = optionparser_v2::nextTokenSuggestionsMulti(
        {command_root1, command_root2}, input_string);

    EXPECT_EQ(suggestions.size(), 1);
    EXPECT_THAT(suggestions, ::testing::Contains("two"));
}

TEST(generateUsageString, CommandWithFlagAndParameter) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("one", "one");
    auto flag = makeFlag("--help", "-h", "Print help message");
    auto command =
        makeCommand("clone", "Clone a repository", {flag, parameter});

    std::string usage_string = optionparser_v2::generateUsageString(command);
    EXPECT_FALSE(usage_string.empty());
}

TEST(generateHelpString, CommandWithFlagAndParameter) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("one", "one");
    auto flag = makeFlag("--help", "-h", "Print help message");
    auto command =
        makeCommand("clone", "Clone a repository", {flag, parameter});

    std::string help_string = optionparser_v2::generateHelpString(command);
    EXPECT_FALSE(help_string.empty());
}
