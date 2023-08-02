// main.cpp

// C++
#include <iostream>
#include <string>

// Local
#include "../include/OptionParser.hpp"

#if 1

int main(int argc, char **argv) {
    using namespace OptionParser;
    Parser parser(
        Option<WordType>("gcc"), Option<>("-c"), Option<>("-S"), Option<>("-E"),
        Option<WordType>("-std"), Option<>("-g"), Option<>("-pg"),
        Option<NumberType<int>>("-O"), Option<ListType>("-W"),
        Option<WordType>("-W"), Option<>("-pedantic"), Option<ListType>("-I"),
        Option<WordType>("-I"), Option<ListType>("-L"), Option<WordType>("-L"),
        Option<ListType>("-D"), Option<WordType>("-D"), Option<WordType>("-U"),
        Option<ListType>("-f"), Option<WordType>("-f"), Option<ListType>("-m"),
        Option<WordType>("-m"), Option<WordType>("-o"));

    std::string input_str = "gcc main.cpp -o main -std c++17 -O 3 -D "
                            "[EXAMPLE_MACRO1=0x1010, EXAMPLE_MACRO2=TEST]";

    auto set = parser.parse(input_str);

    if (auto gcc_result = set.find<0>()) {
        std::cout << "gcc argument: " << gcc_result->get<0>() << "\n";

        if (set.find<1>()) {
            std::cout << "-c option\n";
        } else if (set.find<2>()) {
            std::cout << "-S option\n";
        } else if (set.find<3>()) {
            std::cout << "-E option\n";
        }

        if (auto result_std = set.find<4>()) {
            std::cout << "-std option: " << result_std->get<0>() << "\n";
        }

        if (set.find<5>()) {
            std::cout << "-g option\n";
        }

        if (set.find<6>()) {
            std::cout << "-pg option\n";
        }

        if (auto result_O = set.find<7>()) {
            std::cout << "-O option: " << result_O->get<0>() << "\n";
        }

        if (auto result_W = set.find<8>()) {
            std::cout << "-W option:\n";
            for (auto m : result_W->get<0>()) {
                std::cout << "\t" << m << "\n";
            }
        } else if (auto result_W = set.find<9>()) {
            std::cout << "-W option: " << result_W->get<0>() << "\n";
        }

        if (auto result_pedantic = set.find<10>()) {
            std::cout << "-pedantic option\n";
        }

        if (auto result_I = set.find<11>()) {
            std::cout << "-I option:\n";
            for (auto m : result_I->get<0>()) {
                std::cout << "\t" << m << "\n";
            }
        } else if (auto result_I = set.find<12>()) {
            std::cout << "-I option: " << result_I->get<0>() << "\n";
        }

        if (auto result_L = set.find<13>()) {
            std::cout << "-L option:\n";
            for (auto m : result_L->get<0>()) {
                std::cout << "\t" << m << "\n";
            }
        } else if (auto result_L = set.find<14>()) {
            std::cout << "-L option: " << result_L->get<0>() << "\n";
        }

        if (auto result_D = set.find<15>()) {
            std::cout << "-D option:\n";
            for (auto m : result_D->get<0>()) {
                std::cout << "\t" << m << "\n";
            }
        } else if (auto result_D = set.find<16>()) {
            std::cout << "-D option: " << result_D->get<0>() << "\n";
        }

        if (auto result_U = set.find<17>()) {
            std::cout << "-U option: " << result_U->get<0>() << "\n";
        }

        if (auto result_f = set.find<18>()) {
            std::cout << "-f option:\n";
            for (auto m : result_f->get<0>()) {
                std::cout << "\t" << m << "\n";
            }
        } else if (auto result_f = set.find<19>()) {
            std::cout << "-f option: " << result_f->get<0>() << "\n";
        }

        if (auto result_m = set.find<20>()) {
            std::cout << "-m option:\n";
            for (auto m : result_m->get<0>()) {
                std::cout << "\t" << m << "\n";
            }
        } else if (auto result_m = set.find<21>()) {
            std::cout << "-m option: " << result_m->get<0>() << "\n";
        }

        if (auto result_o = set.find<22>()) {
            std::cout << "-o option: " << result_o->get<0>() << "\n";
        }
    }

    return 0;
}

#endif

#if 0
/*
Overload flags.
*/
int main(int argc, char** argv)
{
	using namespace OptionParser;
	Parser parser(Option<NumberType<int>>("-flag1"), Option<WordType>("-flag1"));

	std::string str1 = "-flag1 12345";
	std::string str2 = "-flag1 word123";

	auto set1 = parser.parse(str1);

	if (auto result = set1.find<0>()) {
		std::cout << "NumberType: " << result->get<0>() << "\n";
	}
	else if (auto result = set1.find<1>()) {
		std::cout << "WordType: " << result->get<0>() << "\n";
	}
	else {
		std::cout << "Not found\n";
	}

	auto set2 = parser.parse(str2);

	if (auto result = set2.find<0>()) {
		std::cout << "NumberType: " << result->get<0>() << "\n";
	}
	else if (auto result = set2.find<1>()) {
		std::cout << "WordType: " << result->get<0>() << "\n";
	}
	else {
		std::cout << "Not found\n";
	}

	return 0;
}

#endif

#if 0
/*
Only capture flag if the input string fulfills the option parameter requirements.
*/
int main(int argc, char** argv)
{
	using namespace OptionParser;
	Parser parser(Option<StringType, NumberType<int>>("-flag1"));

	std::string str1 = "-flag1 -flag1 \"this is a string\" 123 -flag1";
	std::string str2 = "-flag1 -flag1 -flag1 -flag1 -flag1";

	auto set1 = parser.parse(str1);

	if (auto result = set1.find<0>()) {
		std::cout << result->get<0>() << " " << result->get<1>() << "\n";
	}
	else {
		std::cout << "Did not find flag1 in str1\n";
	}

	auto set2 = parser.parse(str2);

	if (auto result = set2.find<0>()) {
		std::cout << result->get<0>() << " " << result->get<1>() << "\n";
	}
	else {
		std::cout << "Did not find flag1 in str2\n";
	}

	return 0;
}

#endif

#if 0

template <typename Iterator>
void print(Iterator begin, Iterator end)
{
	while (begin != end) {
		if (begin < end - 1) {
			std::cout << *begin << ", ";
		}
		else {
			std::cout << *begin << "\n";
		}

		std::advance(begin, 1);
	}
}

int main(int argc, char** argv)
{

	std::cout << std::boolalpha;

	using namespace OptionParser;

	Parser parser(Option<NumberType<int>>("-int"), Option<NumberType<double>>("-double"));

	//std::string_view str1("-int 123.123213 -double -123");
	std::string_view str1("-double -123");
	//std::string_view str1(" a -1 \" -0 sgjerogjer \" -2 -0 b    ");

	auto set = parser.parse(str1);

	if (auto r = set.find<0>()) {
		std::cout << "found 0\n";
		auto l = r->get<0>();
		std::cout << l << "\n";
	}
	if (auto r = set.find<1>()) {
		std::cout << "found 1\n";
		auto l = r->get<0>();
		std::cout << l << "\n";
	}

	//if (auto r = set.find<2>()) {
	//	std::cout << "found 2\n";
	//}
	//if (auto r = set.find<3>()) {
	//	std::cout << "found 3\n";
	//}

	return 0;
}
#endif

#if 0

int main(int argc, char** argv)
{
	OptionParser parser(Option("-0"), Option("-1", 1), Option("-2"));

	auto set = parser.parse("-1 -2 nam -2 -2 -1 -3");

	if (auto r = set.find(parser.get<0>())) {
		std::cout << "found 0" << "\n";
	}
	if (auto r = set.find(parser.get<1>())) {
		std::cout << "found 1" << "\n";
	}
	if (auto r = set.find(parser.get<2>())) {
		std::cout << "found 2" << "\n";
	}




	return 0;
}
#endif