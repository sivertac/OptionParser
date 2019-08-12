
#include <doctest/doctest.h>
#include <unistd.h>
#include <string>
#include <OptionParser.hpp>

TEST_CASE("Create Parser") {
    using namespace OptionParser;
    Parser parser(Option<NumberType<int>>("number"));

    std::string input_string = "number 123";

    auto set = parser.parse(input_string);
    if (auto res = set.find<0>()) {
        CHECK(res->get<0>() == 123);
    }
}


