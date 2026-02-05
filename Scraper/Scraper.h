#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <gumbo.h>
#include <string_view>
#include <algorithm>
#include <fstream>
#include <rapidjson/Document.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <thread>
#include <chrono>
using namespace std;

bool iequals(std::string_view a, std::string_view b);
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp);
int DownloadHTML(const string& url, string* html);
void find_buttons_and_images(const GumboNode* node);
void find_all_links(const GumboNode* node, vector<string>& links);
std::string readAll(const std::string& fileName);