#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include "curl_downloader.h"
#include <curl/curl.h>

// Load key=value pairs from .env and set as environment variables
void loadDotEnv(const std::string& filename = ".env") {
    std::ifstream file(filename);
    if (!file) return;
    std::string line;
    while (std::getline(file, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {
                setenv(key.c_str(), value.c_str(), 1); // 1 = overwrite
            }
        }
    }
}

// Print a separator line for better console formatting
void printSeparator(char c = '-', int width = 80) {
    std::cout << std::string(width, c) << "\n";
}

// Print a formatted header with a title
void printHeader(const std::string& title) {
    printSeparator('=');
    std::cout << "=== " << title << " ===" << "\n";
    printSeparator('=');
}

// Parse command-line arguments
void parseArgs(int argc, char* argv[], std::string& searchTerm, std::vector<std::string>& qualifiers, int& page) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-s" || arg == "--search") && i + 1 < argc) {
            searchTerm = argv[++i];
        } else if (arg == "-q" && i + 1 < argc) {
            qualifiers.push_back(argv[++i]);
        } else if ((arg == "-p" || arg == "--page") && i + 1 < argc) {
            page = std::stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: github-searcher -s \"search term\" [-q \"qualifier\"]... [-p page]\n";
            std::cout << "Example: github-searcher -s \"cpp web server\" -q \"stars:>500\" -q \"language:C++\"\n";
            exit(0);
        }
    }
}

int main(int argc, char* argv[]) {
    loadDotEnv();

    std::string searchTerm;
    std::vector<std::string> qualifiers;
    int page = 1;

    parseArgs(argc, argv, searchTerm, qualifiers, page);

    if (searchTerm.empty()) {
        std::cerr << "Usage: github-searcher -s \"search term\" [-q \"qualifier\"]... [-p page]\n";
        std::cerr << "Example: github-searcher -s \"cpp web server\" -q \"stars:>500\" -q \"language:C++\"\n";
        return 1;
    }

    printHeader("GitHub Repository Search CLI");

    CURLcode global_init_res = curl_global_init(CURL_GLOBAL_ALL);
    if (global_init_res != CURLE_OK) {
        std::cerr << "CRITICAL ERROR: Failed to initialize libcurl globally.\n";
        std::cerr << "libcurl error: " << curl_easy_strerror(global_init_res) << '\n';
        return 1;
    }

    CurlDownloader downloader;

    const char* env_token = std::getenv("GITHUB_TOKEN");
    if (env_token && std::string(env_token).length() > 0) {
        downloader.set_auth_token(std::string(env_token));
        std::cout << "GitHub API token loaded from environment.\n";
    } else {
        std::cout << "No GitHub API token found in environment. You may be rate-limited.\n";
    }

    std::vector<ProjectInfo> found_projects;
    long http_status = downloader.searchRepositories(searchTerm, qualifiers, found_projects, page);

    if (http_status == 200) {
        if (found_projects.empty()) {
            std::cout << "No repositories found matching your criteria.\n";
        } else {
            std::cout << "Found " << found_projects.size() << " repositories.\n";
            printSeparator();
            for (size_t i = 0; i < found_projects.size(); ++i) {
                const auto& project = found_projects[i];
                std::cout << "Result " << (i + 1) << ":\n";
                std::cout << "  Name:        " << project.name << "\n";
                std::cout << "  URL:         " << project.html_url << "\n";
                std::cout << "  Description: " << (project.description.empty() ? "No description provided." : project.description) << "\n";
                std::cout << "  Stars:       " << project.stargazers_count << "\n";
                std::cout << "  Last Push:   " << project.pushed_at << "\n";
                std::cout << "  License:     " << project.license << "\n";
                printSeparator('.');
            }
        }
    } else {
        std::cerr << "GitHub API request failed. HTTP status: " << http_status << "\n";
    }

    curl_global_cleanup();
    return 0;
}
