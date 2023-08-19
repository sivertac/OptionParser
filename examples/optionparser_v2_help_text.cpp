
#include <OptionParser_v2.hpp>
#include <iostream>

int main(int argc, char **argv) {

    using namespace optionparser_v2;

    Component root_component = makeCommand(
        "root", "root command",
        {makeFlag("--flag", "", "flag description"),
         makeParameter("parameter", "parameter description"),
         makeFlag("--flag2", "", "flag2 description",
                  {makeRequiredParameter("flag2_parameter",
                                         "flag2 parameter description")}),
         makeRequiredCommand(
             "subcommand", "subcommand description",
             {makeFlag("--subcommand_flag", "-s",
                       "subcommand flag description"),
              makeParameter("subcommand_parameter",
                            "subcommand parameter description")})});

    std::cout << generateUsageString(root_component) << std::endl;

    std::cout << generateHelpString(root_component) << std::endl;

    return 0;
}
