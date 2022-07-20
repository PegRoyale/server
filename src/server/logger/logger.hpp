#pragma once

#include <iostream>
#include <cstdio>
#include <algorithm>
#include <regex>

#define PRINT_FILE_CONSOLE(__FMT__, ...)												\
	if (logger::file)									\
	{																					\
		std::fprintf(logger::file, __FMT__, __VA_ARGS__);										\
		std::fflush(logger::file);																\
	}																					\
																						\
	std::printf(__FMT__, __VA_ARGS__)

#ifdef DEBUG
#define PRINT_DEBUG(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ DEBUG ][" __FUNCTION__ "]: " __FMT__ "\n", __VA_ARGS__)

#define PRINT_INFO(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ INFO ][" __FUNCTION__ "]: " __FMT__ "\n", __VA_ARGS__)

#define PRINT_WARNING(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ WARNING ][" __FUNCTION__ "]: " __FMT__ "\n", __VA_ARGS__)

#define PRINT_ERROR(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ ERROR ][" __FUNCTION__ "]: " __FMT__ "\n", __VA_ARGS__)
#else
#define PRINT_DEBUG(__FMT__, ...)

#define PRINT_INFO(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ INFO ]: " __FMT__ "\n", __VA_ARGS__)

#define PRINT_WARNING(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ WARNING ]: " __FMT__ "\n", __VA_ARGS__)

#define PRINT_ERROR(__FMT__, ...)													\
		PRINT_FILE_CONSOLE("[ ERROR ]: " __FMT__ "\n", __VA_ARGS__)
#endif

class logger
{
public:
	static _iobuf* file;

	static void init(const char* title)
	{
		if (GetConsoleWindow() == NULL)
		{
			std::ios_base::sync_with_stdio(false);

			file = std::fopen(logger::va("server.log", title).c_str(), "wb");

			AllocConsole();
			SetConsoleTitleA(title);

			std::freopen("CONOUT$", "w", stdout);
			std::freopen("CONIN$", "r", stdin);
		}
	}

	static std::string va(const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		char result[512]{};
		std::vsprintf(result, fmt, va);
		return std::string(result);
	}

	static std::vector<std::string> split(const std::string& s, const std::string& seperator)
	{
		std::vector<std::string> output;

		std::string::size_type prev_pos = 0, pos = 0;

		while ((pos = s.find(seperator, pos)) != std::string::npos)
		{
			std::string substring(s.substr(prev_pos, pos - prev_pos));

			output.push_back(substring);

			prev_pos = ++pos;
		}

		output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word

		return output;
	}

	static void to_lower(std::string& string)
	{
		std::for_each(string.begin(), string.end(), ([](char& c)
		{
			c = std::tolower(c);
		}));
	}

	static bool ends_with(std::string const& value, std::string const& ending)
	{
		if (ending.size() > value.size()) return false;
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}

	static std::string replace(std::string const& in, std::string const& from, std::string const& to)
	{
		return std::regex_replace(in, std::regex(from), to);
	}
};
