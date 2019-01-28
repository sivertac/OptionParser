# OptionParser
A C++17 header only string parser library. 
The goal is to extract simple structure out of strings in a similar fashion to how *nix command line programs use flags to spesify options.
To use the library, spesify how you want the structure of the string to look in the template parameters, and a parser will be generated.
					
Thanks to meta-template-programming, OptionParser will generate a parser based on the parameters at compile time, which allows the parser to be used at runtime with no startup cost, and minimal parse-time cost (we can apply compile time optimizations to the spesific parser).
A spesific implemtation optimization is the use of C++17 std::string_view, to have no heap allocations while parsing (the maximum amount of options is known at compile time). There are no stored copies of parsed results, only pointers to the original string. The rationale for this optimization is to reduce size and improve cache reuse and locality.
The library is statically typed, which means that you will get a compile time error if you try to access a result member(option) that is not part of the parser generated.

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

	