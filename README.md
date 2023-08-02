# OptionParser
A C++17 string parser library. 
The goal is to extract simple structure out of strings in a similar fashion to how *nix command line programs use flags to specify options.
To use the library, specify how you want the structure of the string to look in the template parameters, and a parser will be generated.
					
Thanks to meta-template-programming, OptionParser will generate a parser based on the parameters at compile time, which allows the parser to be used at runtime with no startup cost, and minimal parse-time cost (we can apply compile time optimizations to the spesific parser).
A spesific implemtation optimization is the use of C++17 std::string_view, to have no heap allocations while parsing (the maximum amount of options is known at compile time). There are no stored copies of parsed results, only pointers to the original string. The rationale for this optimization is to reduce size and improve cache reuse and locality.
The library is statically typed, which means that you will get a compile time error if you try to access a result member(option) that is not part of the parser generated.

## Requirements
C++17 or later compatible compiler.

## Examples
How a mini version of the gcc command (man gcc) would be implemented in OptionParser. (In some of the gcc options, the option and arguments are not separated. OptionParser is always using spaces.)
(It is not possible to have multiple instances the same option type in OptionParser, so the "-D" option can not be properly implemented in OptionParser at this time, instead the option is implemented as a ListType and then overloaded with a WordType.
```cpp
using namespace OptionParser;
Parser parser(
	Option<WordType>("gcc"),
	Option<>("-c"),
	Option<>("-S"),
	Option<>("-E"),
	Option<WordType>("-std"),
	Option<>("-g"),
	Option<>("-pg"),
	Option<NumberType<int>>("-O"),
	Option<ListType>("-W"),
	Option<WordType>("-W"),
	Option<>("-pedantic"),
	Option<ListType>("-I"),
	Option<WordType>("-I"),
	Option<ListType>("-L"),
	Option<WordType>("-L"),
	Option<ListType>("-D"),
	Option<WordType>("-D"),
	Option<WordType>("-U"),
	Option<ListType>("-f"),
	Option<WordType>("-f"),
	Option<ListType>("-m"),
	Option<WordType>("-m"),
	Option<WordType>("-o")
);

//try to change this string
std::string input_str = "gcc main.cpp -o main -std c++17 -O 3 -D [EXAMPLE_MACRO1=0x1010, EXAMPLE_MACRO2=TEST]";

auto set = parser.parse(input_str);

if (auto gcc_result = set.find<0>()) {
	std::cout << "gcc argument: " << gcc_result->get<0>() << "\n";

	if (set.find<1>()) {
		std::cout << "-c option\n";
	}
	else if (set.find<2>()) {
		std::cout << "-S option\n";
	}
	else if (set.find<3>()) {
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
	}
	else if (auto result_W = set.find<9>()) {
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
	}
	else if (auto result_I = set.find<12>()) {
		std::cout << "-I option: " << result_I->get<0>() << "\n";
	}

	if (auto result_L = set.find<13>()) {
		std::cout << "-L option:\n";
		for (auto m : result_L->get<0>()) {
			std::cout << "\t" << m << "\n";
		}
	}
	else if (auto result_L = set.find<14>()) {
		std::cout << "-L option: " << result_L->get<0>() << "\n";
	}

	if (auto result_D = set.find<15>()) {
		std::cout << "-D option:\n";
		for (auto m : result_D->get<0>()) {
			std::cout << "\t" << m << "\n";
		}
	}
	else if (auto result_D = set.find<16>()) {
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
	}
	else if (auto result_f = set.find<19>()) {
		std::cout << "-f option: " << result_f->get<0>() << "\n";
	}

	if (auto result_m = set.find<20>()) {
		std::cout << "-m option:\n";
		for (auto m : result_m->get<0>()) {
			std::cout << "\t" << m << "\n";
		}
	}
	else if (auto result_m = set.find<21>()) {
		std::cout << "-m option: " << result_m->get<0>() << "\n";
	}

	if (auto result_o = set.find<22>()) {
		std::cout << "-o option: " << result_o->get<0>() << "\n";
	}

}
```
```
Output:
gcc argument: main.cpp
-std option: c++17
-O option: 3
-D option:
        EXAMPLE_MACRO1=0x1010
        EXAMPLE_MACRO2=TEST
-o option: main
```

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

	