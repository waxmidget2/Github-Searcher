#ifndef CURL_DOWNLOADER_H
#define CURL_DOWNLOADER_H

#include <string>
#include <vector>
#include <curl/curl.h>
#include "json.hpp"

struct ProjectInfo {
    std::string name;
    std::string html_url;
    std::string description;
    std::string pushed_at;
    int stargazers_count;
};

class CurlDownloader {
public:
    CurlDownloader();
    ~CurlDownloader();
    void set_auth_token();
    void download_url(const std::string& url, const std::string& name);
    long searchRepositories(const std::string& search_term,
                            const std::vector<std::string>& qualifiers,
                            std::vector<ProjectInfo>& projects_out,
                            int page);

private:
    CURL* curl_handle;
    std::string auth_token;
    static size_t write_callback_std_string(void* contents, size_t size, size_t nmemb, std::string* s);
    std::string urlEncode(const std::string& str_to_encode);
};

#endif
