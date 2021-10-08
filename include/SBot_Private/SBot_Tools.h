#pragma once

#include <string>
#include <vector>

std::string gen_echo();

std::string to_ansi(const std::string & utf8_str);

std::string to_utf8(const std::string & ansi_str);

std::string url_encode(const std::string& url);

std::vector<std::vector<std::string>> match_all(const std::string& content, const std::string& pattern);