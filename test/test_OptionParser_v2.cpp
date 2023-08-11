
#include <OptionParser_v2.cpp>
#include <OptionParser_v2.hpp>
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

TEST(parseIncomplete, EmptyString) {
    using namespace optionparser_v2;
    auto flag = makeFlag("help", "h", "Print help message", {});

    std::string_view input_string = "";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(flag, input_string);
    EXPECT_FALSE(result.has_value());
}

TEST(parseIncomplete, ParameterOneWord) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("url", "URL of repository");

    std::string_view input_string = "randomurl";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(parameter, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "randomurl");
    EXPECT_TRUE(result->m_component->isParameter());
}

TEST(parseIncomplete, FlagOneWord) {
    using namespace optionparser_v2;
    auto flag = makeFlag("help", "h", "Print help message", {});

    std::string_view input_string = "--help";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(flag, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "--help");
    EXPECT_TRUE(result->m_component->isFlag());
}

TEST(parseIncomplete, PositionalIdentifierOneWord) {
    using namespace optionparser_v2;
    auto positional_identifier =
        makePositionalIdentifier("clone", "Clone a repository", {});

    std::string_view input_string = "clone";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(positional_identifier, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "clone");
    EXPECT_TRUE(result->m_component->isPositionalIdentifier());
}

TEST(parseIncomplete, PositionalIdentifierContainingParameterAndFlag) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("url", "URL of repository");
    auto flag = makeFlag("help", "h", "Print help message", {});
    std::vector<Component> children;
    children.push_back(std::move(parameter));
    children.push_back(std::move(flag));
    auto positional_identifier = makePositionalIdentifier(
        "clone", "Clone a repository", std::move(children));

    std::string_view input_string = "clone --help yo";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(positional_identifier, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "clone");
    EXPECT_EQ(result->m_component, &positional_identifier);
    EXPECT_EQ(result->m_children.size(), 2);
    EXPECT_EQ(result->m_children[0].m_value, "--help");
    EXPECT_TRUE(result->m_children[0].m_component->isFlag());
    EXPECT_EQ(result->m_children[1].m_value, "yo");
    EXPECT_TRUE(result->m_children[1].m_component->isParameter());
}

TEST(parseIncomplete,
     PositionalIdentifierContainingParameterAndFlagAndPositionalIdentifier) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("url", "URL of repository");
    auto flag = makeFlag("help", "h", "Print help message", {});
    auto positional_identifier =
        makePositionalIdentifier("clone", "Clone a repository", {});
    std::vector<Component> children;
    children.push_back(std::move(parameter));
    children.push_back(std::move(flag));
    children.push_back(std::move(positional_identifier));
    auto positional_identifier2 = makePositionalIdentifier(
        "pull", "Pull a repository", std::move(children));

    std::string_view input_string = "pull clone --help yo";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(positional_identifier2, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "pull");
    EXPECT_EQ(result->m_component, &positional_identifier2);
    EXPECT_EQ(result->m_children.size(), 3);
    EXPECT_EQ(result->m_children[0].m_value, "clone");
    EXPECT_TRUE(result->m_children[0].m_component->isPositionalIdentifier());
    EXPECT_EQ(result->m_children[1].m_value, "--help");
    EXPECT_TRUE(result->m_children[1].m_component->isFlag());
    EXPECT_EQ(result->m_children[2].m_value, "yo");
    EXPECT_TRUE(result->m_children[2].m_component->isParameter());
}

TEST(serializeResult, TwoParameters) {
    using namespace optionparser_v2;
    auto parameter1 =
        makeParameter("one", "one", {makeParameter("two", "two")});

    std::string_view input_string = "one two";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(parameter1, input_string);
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
    auto flag = makeFlag("help", "h", "Print help message", {parameter1});

    std::string_view input_string = "--help one two";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(flag, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "--help");
    EXPECT_TRUE(result->m_component->isFlag());

    std::string serialized = optionparser_v2::serializeResult(*result);
    EXPECT_EQ(serialized, input_string);
}

TEST(serializeResult, PositionalIdentifierWithFlagAndParameter) {
    using namespace optionparser_v2;
    auto parameter = makeParameter("one", "one");
    auto flag = makeFlag("help", "h", "Print help message");
    auto positional_identifier = makePositionalIdentifier(
        "clone", "Clone a repository", {flag, parameter});

    std::string_view input_string = "clone one --help";
    std::optional<ParseResult> result =
        optionparser_v2::parseIncomplete(positional_identifier, input_string);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->m_value, "clone");
    EXPECT_TRUE(result->m_component->isPositionalIdentifier());

    std::string serialized = optionparser_v2::serializeResult(*result);
    EXPECT_EQ(serialized, input_string);
}
