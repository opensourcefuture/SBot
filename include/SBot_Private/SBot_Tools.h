#pragma once

#include <string>

std::string gen_echo();

std::string to_ansi(const std::string & utf8_str);

std::string to_utf8(const std::string & ansi_str);

std::string url_encode(const std::string& url);