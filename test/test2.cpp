
#include <OptionParser.hpp>
#include <string>
#include <unistd.h>

#include <gtest/gtest.h>

TEST(OptionParser, test1) {
    using namespace OptionParser;
    Parser parser(Option<NumberType<int>>("number"));

    std::string input_string = "number 123";

    auto set = parser.parse(input_string);
    if (auto res = set.find<0>()) {
        EXPECT_EQ(res->get<0>(), 123);
    }
}
