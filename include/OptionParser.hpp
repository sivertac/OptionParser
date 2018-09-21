//OptionParser.hpp
//Author: Sivert Andresen Cubedo

#pragma once
#ifndef OptionParser_HEADER
#define OptionParser_HEADER

#if 1


//C++
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <istream>
#include <tuple>
#include <functional>
#include <array>
#include <utility>
#include <cassert>
#include <type_traits>

namespace OptionParser
{

	/*
	Split string by spaces.
	*/
	std::vector<std::string_view> extractWords(std::string_view str_view, const std::string_view & delim = " \t")
	{
		std::vector<std::string_view> word_vec;
		while (!str_view.empty()) {
			std::size_t delim_pos = str_view.find_first_of(delim);
			std::size_t word_pos = str_view.find_first_not_of(delim);
			if (delim_pos == str_view.npos) {
				word_vec.push_back(str_view);
				break;
			}
			else if (word_pos == str_view.npos) {
				break;
			}
			else if (delim_pos > word_pos) {
				word_vec.push_back(str_view.substr(0, delim_pos));
				str_view.remove_prefix(delim_pos);
			}
			else {
				str_view.remove_prefix(word_pos);
			}
		}
		return word_vec;
	}

	/*
	Remove prefix delim.
	*/
	std::pair<std::string_view, std::size_t> removePrefixDelim(std::string_view str_view, const std::string_view & delim = " \t")
	{
		auto delim_pos = str_view.find_first_of(delim);
		if (delim_pos == str_view.npos) return std::make_pair(str_view, 0);
		auto word_pos = str_view.find_first_not_of(delim);
		if (word_pos == str_view.npos) {
			std::size_t s = str_view.size();
			str_view.remove_prefix(str_view.size());
			return std::make_pair(str_view, s);
		}
		if (word_pos < delim_pos) {
			return std::make_pair(str_view, 0);
		}
		else {
			str_view.remove_prefix(word_pos);
			return std::make_pair(str_view, word_pos);
		}
	}

	/*
	Get first word.
	*/
	std::pair<std::string_view, std::size_t> extractFirstWord(std::string_view str_view, const std::string_view & delim = " \t")
	{
		auto pair = removePrefixDelim(str_view, delim);
		auto delim_pos = pair.first.find_first_of(delim);
		if (delim_pos == pair.first.npos) return std::make_pair(pair.first, pair.second + pair.first.size());
		auto word_pos = pair.first.find_first_not_of(delim);
		if (word_pos == pair.first.npos) {
			str_view.remove_prefix(pair.first.size());
			return std::make_pair(pair.first, pair.second);
		}
		return std::make_pair(pair.first.substr(0, delim_pos), pair.second + delim_pos);
	}

	/*
	Check if string has delim before first not delim.
	*/
	bool hasPrefixDelim(std::string_view str_view, const std::string_view & delim = " \t")
	{
		auto delim_pos = str_view.find_first_of(delim);
		auto word_pos = str_view.find_first_not_of(delim);
		if (delim_pos == str_view.npos && word_pos == str_view.npos) {
			return false;
		}
		else if (delim_pos == str_view.npos) {
			return false;
		}
		else if (word_pos == str_view.npos) {
			return true;
		}
		else if (delim_pos > word_pos) {
			return false;
		}
		else /* if (delim_pos < word_pos) */ {
			return true;
		}
	}

	/*
	Remove first word (and delims).
	*/
	std::pair<std::string_view, std::size_t> removeFirstWord(std::string_view str_view, const std::string_view & delim = " \t")
	{
		std::size_t s = 0;
		auto pair = removePrefixDelim(str_view, delim);
		s += pair.second;
		auto first_word = extractFirstWord(pair.first, delim);
		pair.first.remove_prefix(first_word.first.size());
		s += first_word.first.size();
		pair = removePrefixDelim(pair.first, delim);
		s += pair.second;
		return std::make_pair(pair.first, s);
	}

	/*
	Search for end of list.
	(first char must be start of list).
	*/
	std::size_t searchEndOfList(std::string_view str_view, char begin = '[', char end = ']')
	{
		assert(begin != end);

		if (str_view.empty()) return str_view.npos;

		str_view.remove_prefix(1); //remove first begin
		std::size_t count = 1;

		std::size_t total_pos = 0;

		while (count > 0) {
			auto end_pos = str_view.find(end);
			if (end_pos == str_view.npos) return str_view.npos;
			auto begin_pos = str_view.find(begin);
			if (begin_pos < end_pos) {
				str_view.remove_prefix(begin_pos + 1);
				total_pos += begin_pos + 1;
				++count;
			}
			else {
				str_view.remove_prefix(end_pos + 1);
				total_pos += end_pos + 1;
				--count;
			}
		}
		return total_pos;
	}

	/*
	Extract list from string.
	*/
	std::pair<std::string_view, std::size_t> extractFirstList(std::string_view str_view, char begin = '[', char end = ']', const std::string_view & delim = " \t")
	{
		assert(begin != end);
		auto pair = removePrefixDelim(str_view, delim);
		auto end_pos = searchEndOfList(pair.first, begin, end);
		if (end_pos == pair.first.npos) {
			return std::make_pair(pair.first.substr(0, 0), pair.second);
		}
		else {
			return std::make_pair(pair.first.substr(0, end_pos + 1), pair.second + end_pos + 1);
		}
	}

	/*
	Parse extracted list.
	*/
	std::vector<std::string_view> parseList(std::string_view str_view, const std::string_view & delim = " \t")
	{
		str_view.remove_prefix(1);
		str_view.remove_suffix(1);
		return extractWords(str_view, delim);
	}

	/*
	Extract string.
	*/
	std::pair<std::string_view, std::size_t> extractFirstString(std::string_view str_view, const std::string_view & delim = " \t")
	{
		auto pair = removePrefixDelim(str_view, delim);

		if (pair.first.empty() || pair.first.front() != '"') {
			return std::make_pair(pair.first.substr(0, 0), pair.second);
		}

		pair.first.remove_prefix(1);
		auto end = pair.first.find_first_of('"');
		if (end == pair.first.npos) {
			return std::make_pair(pair.first.substr(0, 0), pair.second + 1);
		}
		return std::make_pair(pair.first.substr(0, end), pair.second + 1 + end + 1);
	}

	template <class Tup, class Func, std::size_t ...Is>
	constexpr void static_for_impl(Tup& t, Func &&f, std::index_sequence<Is...>)
	{
		(f(std::ref(std::get<Is>(t))), ...);
	}

	template <class ... T, class Func >
	constexpr void static_for(std::tuple<T...>&t, Func &&f)
	{
		static_for_impl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)>{});
	}

	struct WordType { std::string_view type; };
	struct ListType { std::vector<std::string_view> type; };
	struct StringType { std::string_view type; };

	template <class ... Params>
	class Option;

	template <class ... Params>
	class Result
	{
	public:
		using OptionType = Option<Params...>;
		Result() = default;

		template <std::size_t i>
		constexpr auto & get() const
		{
			return std::get<i>(m_params).type;
		}

		constexpr std::size_t getSize() const
		{
			return sizeof...(Params);
		}
	private:
		friend OptionType;
		std::tuple<Params...> m_params;
	};

	template <class ... Params>
	class Option
	{
	public:
		using ResultType = Result<Params...>;

		Option(const std::vector<std::string_view> & identifiers) :
			m_identifiers(identifiers)
		{
		}

		Option(const std::string_view & identifier) :
			Option(std::vector<std::string_view>{ identifier })
		{
		}

		bool checkIdentifier(const std::string_view & str) const
		{
			auto it = std::find_if(m_identifiers.begin(), m_identifiers.end(), [&](auto & id) {return str == id; });
			if (it == m_identifiers.end()) {
				return false;
			}
			else {
				return true;
			}
		}

		std::optional<ResultType> parse(std::string_view & input_view) const
		{
			if (input_view.empty()) return std::nullopt;
			const std::string_view delim(" \t,");
			std::string_view temp_view = input_view;
			//check if identifier is correct
			auto pair = extractFirstWord(temp_view, delim);
			if (!checkIdentifier(pair.first)) return std::nullopt;
			temp_view.remove_prefix(pair.second);

			if (sizeof...(Params) == 0) {
				input_view = temp_view;
				return ResultType();
			}

			//recursive parse
			ResultType result;
			if (parseParams(result, temp_view)) {
				input_view = temp_view;
				return result;
			}
			else {
				return std::nullopt;
			}
		}

		constexpr std::size_t getSize() const
		{
			return sizeof...(Params);
		}

		constexpr auto & getIdentifiers() const
		{
			return m_identifiers;
		}
	private:
		bool parseParams(ResultType & result, std::string_view & str_view) const
		{
			std::size_t count = result.getSize();
			static_for(result.m_params, [&](auto e) {parseType(e, str_view, count); });
			return (count == 0) ? true : false;
		}

		static void parseType(WordType & ref, std::string_view & str_view, std::size_t & count)
		{
			auto pair = extractFirstWord(str_view);
			if (!pair.first.empty()) {
				ref.type = pair.first;
				str_view.remove_prefix(pair.second);
				--count;
			}
		}
		static void parseType(ListType & ref, std::string_view & str_view, std::size_t & count)
		{
			auto pair = extractFirstList(str_view);
			if (!pair.first.empty()) {
				ref.type = parseList(pair.first);
				str_view.remove_prefix(pair.second);
				--count;
			}
		}
		static void parseType(StringType & ref, std::string_view & str_view, std::size_t & count)
		{
			auto pair = extractFirstString(str_view);
			if (!pair.first.empty()) {
				ref.type = pair.first;
				str_view.remove_prefix(pair.second);
				--count;
			}
		}
	
		std::vector<std::string_view> m_identifiers;
	};

	template <typename T1 = void, typename T2 = void, typename ... Ts>
	constexpr bool allSame()
	{
		if (std::is_void<T2>::value) {
			return true;
		}
		else if (std::is_same<T1, T2>::value) {
			return allSame<T2, Ts...>();
		}
		else {
			return false;
		}
	}

	template <class Param>
	struct OptionResult;

	template <class ... Params>
	struct OptionResult<Option<Params...>>
	{
		using Result = Result<Params...>;
	};

	template <class ... Options>
	class Parser;

	template <class ... Options>
	class ResultSet
	{
	public:
		using ResultTuple = std::tuple<std::pair<bool, typename OptionResult<Options>::Result>...>;

		ResultSet() :
			m_results(std::pair(false, typename OptionResult<Options>::Result())...)
		{
		}

		template <std::size_t i>
		std::optional<typename std::tuple_element<i, ResultTuple>::type::second_type> find() const
		{
			if (std::get<i>(m_results).first) {
				return std::get<i>(m_results).second;
			}
			else {
				return std::nullopt;
			}
		}
	private:
		friend Parser<Options...>;
		ResultTuple m_results;
	};

	template <class ... Options>
	class Parser
	{
	public:
		using OptionTuple = std::tuple<Options...>;
		using ResultSetType = ResultSet<Options...>;

		Parser(Options ... options) :
			m_options(options...)
		{
		}
	
		ResultSetType parse(std::string_view input_line) const
		{
			
			ResultSetType r;
			if (input_line.empty()) return r;

			auto lamb_tup = std::make_tuple(
				[&](const Options & opt, std::pair<bool, typename OptionResult<Options>::Result> & pair) ->bool 
			{
				if (!pair.first) {
					if (auto r = opt.parse(input_line)) {
						pair.first = true;
						pair.second = *r;
						return true;
					}
					else {
						return false;
					}
				}
				else return false;
			}...);

			while (!input_line.empty()) {
				bool found = traverseOptions(lamb_tup, r, std::make_index_sequence<sizeof...(Options)>{});
				if (!found) {
					input_line = removeFirstWord(input_line).first;
				}
			}

			return r;
		}
	
		constexpr const OptionTuple & getOptions() const
		{
			return m_options;
		}
	
	private:
		template <class LambTup, std::size_t ... Is>
		bool traverseOptions(const LambTup & lamb_tup, ResultSetType & r_set, std::index_sequence<Is...>) const
		{
			bool found = false;
			([&]() {
				if (!found) {
					found = std::get<Is>(lamb_tup)(std::get<Is>(m_options), std::get<Is>(r_set.m_results));
				}
			}(), ...);
			return found;
		}

		OptionTuple m_options;
	};
	
}


#endif

#if 0

//C++
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <regex>
#include <optional>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <istream>
#include <tuple>
#include <functional>
#include <array>
#include <utility>

class Result;
class Option;

class Result
{
public:
	Result(const Option & option, std::vector<std::string> && args) :
		m_option(option),
		m_args(std::move(args))
	{
	}
	const Option & getOption() const
	{
		return m_option;
	}
	const std::vector<std::string> & getArgs() const
	{
		return m_args;
	}
private:
	const Option & m_option;
	std::vector<std::string> m_args;
};

class ResultSet
{
public:
	/*
	*/
	ResultSet(std::vector<Result> && vec) :
		m_container(std::move(vec))
	{
	}
	
	/*
	Search for option.
	*/
	std::optional<std::reference_wrapper<Result>> find(const Option & opt)
	{
		auto it = std::find_if(m_container.begin(), m_container.end(), [&](auto & res) {return &res.getOption() == &opt; });
		if (it != m_container.end()) {
			return *it;
		}
		else {
			return std::nullopt;
		}
	}

	/*
	Get m_container.
	*/
	const std::vector<Result> & getVec()
	{
		return m_container;
	}

private:
	std::vector<Result> m_container;
};

class Option
{
public:
	Option(const std::vector<std::string> & identifiers, int args = 0, const std::string & help = "") :
		m_identifiers(identifiers),
		m_args(args),
		m_help(help)
	{
	}

	Option(const std::string & identifier, int args = 0, const std::string & help = "") :
		Option(std::vector<std::string>{identifier}, args, help)
	{
	}
	
	bool checkIdentifier(const std::string & str) const
	{
		auto it = std::find_if(m_identifiers.begin(), m_identifiers.end(), [&](auto & id) {return str == id; });
		if (it == m_identifiers.end()) {
			return false;
		}
		else {
			return true;
		}
	}

	std::optional<Result> parse(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end) const
	{
		int vec_size = static_cast<int>(std::distance(begin, end));
		if (vec_size < m_args + 1) {
			return std::nullopt;
		}
		if (m_args == 0) {
			return Result(*this, std::vector<std::string>{});
		}
		else {
			return Result(*this, std::vector<std::string>{begin + 1, begin + m_args + 1});
		}
	}

	std::optional<Result> parse(std::vector<std::string> & in_vec) const
	{
		return parse(in_vec.begin(), in_vec.end());
	}

	int getSize() const
	{
		return m_args + 1;
	}

	auto & getIdentifiers() const
	{
		return m_identifiers;
	}

private:
	const std::vector<std::string> m_identifiers;
	int m_args;
	std::string m_help;
};

template <typename T1 = void, typename T2 = void, typename ... Ts>
constexpr bool allSame()
{
	if (std::is_void<T2>::value) {
		return true;
	}
	else if (std::is_same<T1, T2>::value) {
		return allSame<T2, Ts...>();
	}
	else {
		return false;
	}
}

template <class ... Os>
class OptionParser
{
public:
	static_assert(allSame<Option, Os...>(), "All parameters must be of type Option");

	using OptionContainer = std::array<Option, sizeof...(Os)>;

	OptionParser(Os ... os) :
		m_options{ os... }
	{
	}

	ResultSet parse(std::vector<std::string> & in_vec) const
	{
		std::vector<Result> result_vec;
		if (in_vec.empty()) return result_vec;

		auto begin = in_vec.begin();
		auto end = in_vec.end();

		while (begin != end) {
			const std::string & str = *begin;
			auto option_it = std::find_if(m_options.begin(), m_options.end(), [&](auto & opt) {return opt.checkIdentifier(str); });
			if (option_it == m_options.end()) {
				//option does not exist
				++begin;
			}
			else {
				//parse option
				if (auto result = option_it->parse(begin, end)) {
					result_vec.push_back(*result);
					begin += result_vec.back().getOption().getSize();
				}
				else {
					++begin;
				}
			}
		}
		return ResultSet(std::move(result_vec));
	}
	ResultSet parse(const std::string & line) const
	{
		std::istringstream iss(line);
		std::vector<std::string> vec{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
		return parse(vec);
	}

	/*
	Get optons.
	*/
	const OptionContainer & getOptions() const
	{
		return m_options;
	}

	/*
	Get option by index.
	*/
	template <std::size_t I>
	constexpr const Option & get() const
	{
		return std::get<I>(m_options);
	}
private:
	std::array<Option, sizeof...(Os)> m_options;
};

#endif

#endif // !OptionParser_HEADER
