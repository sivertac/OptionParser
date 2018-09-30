# OptionParser
A statically typed header only library for parsing option flags.

## Requirements
C++17

## Examples
Only capture flag if the input string fulfills the option parameter requirements. 
```cpp
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
```
	Output:
	this is a string 123
	Did not find flag1 in str2

Overload flags.
```cpp
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
```
	Output:
	NumberType: 12345
	WordType: word123

	