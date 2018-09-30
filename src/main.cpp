//main.cpp

//C++
#include <iostream>
#include <string>

//Local
#include "../include/OptionParser.hpp"

#if 1
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