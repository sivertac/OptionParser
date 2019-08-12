// OptionParser.hpp
// Author: Sivert Andresen Cubedo

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
#include <charconv>

namespace OptionParser
{
#define OptionParser_DEFAULT_DELIM " \t,"

	/*
	Split string by spaces.
	*/
	inline std::vector<std::string_view> extractWords(std::string_view str_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM)
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
	inline std::pair<std::string_view, std::size_t> removePrefixDelim(std::string_view str_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM)
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
	inline std::pair<std::string_view, std::size_t> extractFirstWord(std::string_view str_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM)
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
	inline bool hasPrefixDelim(std::string_view str_view, const std::string_view & delim = " \t")
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
	inline std::pair<std::string_view, std::size_t> removeFirstWord(std::string_view str_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM)
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
	Try to extract first word.
	If succesful then remove word from str_view.
	*/
	inline std::optional<std::string_view> extractFirstWordDestructive(std::string_view & str_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM)
	{
		auto pair1 = extractFirstWord(str_view, delim);
		if (pair1.first.empty()) {
			return std::nullopt;
		}
		else {
			auto pair2 = removeFirstWord(str_view, delim);
			str_view = pair2.first;
			return pair1.first;
		}
	}
	/*
	Search for end of list.
	(first char must be start of list).
	*/
	inline std::size_t searchEndOfList(std::string_view str_view, char begin = '[', char end = ']')
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
	inline std::pair<std::string_view, std::size_t> extractFirstList(std::string_view str_view, char begin = '[', char end = ']', const std::string_view & delim = OptionParser_DEFAULT_DELIM)
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
	inline std::vector<std::string_view> parseList(std::string_view str_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM)
	{
		str_view.remove_prefix(1);
		str_view.remove_suffix(1);
		return extractWords(str_view, delim);
	}

	/*
	Extract string.
	*/
	inline std::pair<std::string_view, std::size_t> extractFirstString(std::string_view str_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM)
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

	struct WordType 
	{ 
		bool parseType(std::string_view & str_view, const std::string_view & delim)
		{
			auto pair = extractFirstWord(str_view, delim);
			if (!pair.first.empty()) {
				type = pair.first;
				str_view.remove_prefix(pair.second);
				return true;
			}
			else {
				return false;
			}
		}
		std::string_view type;
	};
	struct ListType 
	{ 
		bool parseType(std::string_view & str_view, const std::string_view & delim)
		{
			auto pair = extractFirstList(str_view, '[', ']', delim);
			if (!pair.first.empty()) {
				type = parseList(pair.first);
				str_view.remove_prefix(pair.second);
				return true;
			}
			else {
				return false;
			}
		}
		std::vector<std::string_view> type;
	};
	struct StringType 
	{ 
		bool parseType(std::string_view & str_view, const std::string_view & delim)
		{
			auto pair = extractFirstString(str_view, delim);
			if (!pair.first.empty()) {
				type = pair.first;
				str_view.remove_prefix(pair.second);
				return true;
			}
			else {
				return false;
			}
		}
		std::string_view type; 
	};
	template <typename T>
	struct NumberType 
	{
		bool parseType(std::string_view & str_view, const std::string_view & delim)
		{
			auto pair = extractFirstWord(str_view, delim);
			if (!pair.first.empty()) {
				//try to parse number
				auto ret = std::from_chars(pair.first.data(), pair.first.data() + pair.first.size(), type);
				if (ret.ec == std::errc::invalid_argument) {
					return false;
				}
				str_view.remove_prefix(pair.second);
				return true;
			}
			else {
				return false;
			}
		}
		T type;
	};

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

		std::optional<ResultType> parse(std::string_view & input_view, const std::string_view & delim = OptionParser_DEFAULT_DELIM) const
		{
			if (input_view.empty()) return std::nullopt;
			std::string_view temp_view = input_view;
			//check if identifier is correct
			auto pair = extractFirstWord(temp_view, delim);
			if (!checkIdentifier(pair.first)) return std::nullopt;
			temp_view.remove_prefix(pair.second);

			if (sizeof...(Params) == 0) {
				input_view = temp_view;
				return ResultType();
			}

			//check and parse params
			ResultType result;
			if (parseParams(result, temp_view, delim)) {
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
		template <std::size_t ... Is>
		bool traverseParams(ResultType & result, std::string_view & str_view, const std::string_view & delim, std::index_sequence<Is...>) const
		{
			bool failed = false;
			([&](auto & type) 
			{
				if (!failed) {
					failed = !type.parseType(str_view, delim);
				}
			}
			(std::get<Is>(result.m_params)), ...);
			return !failed;
		}

		bool parseParams(ResultType & result, std::string_view & str_view, const std::string_view & delim) const
		{
			return traverseParams(result, str_view, delim, std::make_index_sequence<sizeof...(Params)>{});
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
		using Type = Result<Params...>;
	};

	template <class ... Options>
	class Parser;

	template <class ... Options>
	class ResultSet
	{
	public:
		using ResultTuple = std::tuple<std::pair<bool, typename OptionResult<Options>::Type>...>;

		ResultSet() :
			m_results(std::pair(false, typename OptionResult<Options>::Type())...)
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
	
		ResultSetType parse(std::string_view input_line, const std::string_view & delim = OptionParser_DEFAULT_DELIM) const
		{
			
			ResultSetType r;
			if (input_line.empty()) return r;

			auto lamb_tup = std::make_tuple(
				[&](const Options & opt, std::pair<bool, typename OptionResult<Options>::Type> & pair) ->bool 
			{
				if (!pair.first) {
					if (auto r = opt.parse(input_line, delim)) {
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
					input_line = removeFirstWord(input_line, delim).first;
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

#endif // !OptionParser_HEADER
