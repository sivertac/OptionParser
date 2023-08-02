
#include <OptionParser.hpp>
#include <string>
#include <unistd.h>

#include <gtest/gtest.h>

TEST(OptionParser, WordType) {
    using namespace OptionParser;
    Parser parser(Option<WordType>("word"));

    std::string input_string = "word hello";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res->get<0>(), "hello");
}

TEST(OptionParser, WordTypeInvalid) {
    using namespace OptionParser;
    Parser parser(Option<WordType>("word"));

    std::string input_string = "word ";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_FALSE(res.has_value());
}

TEST(OptionParser, ListType) {
    using namespace OptionParser;
    Parser parser(Option<ListType>("list"));

    std::string input_string = "list [hello, world]";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res->get<0>(), std::vector<std::string_view>({"hello", "world"}));
}

TEST(OptionParser, ListTypeInvalid) {
    using namespace OptionParser;
    Parser parser(Option<ListType>("list"));

    std::string input_string = "list [hello, world";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_FALSE(res.has_value());
}

TEST(OptionParser, StringType) {
    using namespace OptionParser;
    Parser parser(Option<StringType>("string"));

    std::string input_string = "string \"hello world\"";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res->get<0>(), "hello world");
}

TEST(OptionParser, StringTypeInvalid) {
    using namespace OptionParser;
    Parser parser(Option<StringType>("string"));

    std::string input_string = "string \"hello world";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_FALSE(res.has_value());
}

TEST(OptionParser, NumberType) {
    using namespace OptionParser;
    Parser parser(Option<NumberType<int>>("number"));

    std::string input_string = "number                   123    ";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res->get<0>(), 123);
}

TEST(OptionParser, NumberTypeInvalid) {
    using namespace OptionParser;
    Parser parser(Option<NumberType<int>>("number"));

    std::string input_string = "number notnumber";

    auto set = parser.parse(input_string);
    auto res = set.find<0>();
    EXPECT_FALSE(res.has_value());
}
