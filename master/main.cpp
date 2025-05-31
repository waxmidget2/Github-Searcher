#include <iostream>
#include <vector>
#include <string>
#include <limits>    
#include <iomanip>   
#include <algorithm> 

#include "curl_downloader.h" 
#include <curl/curl.h>

void printSeparator(char c = '-', int width = 80) {
    std::cout << std::string(width, c) << "\n";
}

void printHeader(const std::string& title) {
    printSeparator('=');
    std::cout << "=== " << title << " ===" << "\n";
    printSeparator('=');
}

void printSubHeader(const std::string& title) {
    std::cout << "\n--- " << title << " ---" << "\n";
}

void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (std::cin.fail()) { 
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

std::string getNonEmptyLine(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        if (!input.empty()) {
            return input;
        }
        std::cout << "Input cannot be empty. Please try again." << "\n";
    }
}


int main() {
  std::vector<ProjectInfo> found_projects;
  std::string searchTerm {};
  std::vector<std::string> qualifiers;
  printHeader("GitHub Repository Search Client");

  // libcurl global application initialization
  std::cout << "\nInitializing network components (libcurl)..." << "\n";
  CURLcode global_init_res = curl_global_init(CURL_GLOBAL_ALL);
  if (global_init_res != CURLE_OK) {
      printSeparator('!');
      std::cerr << "CRITICAL ERROR: Failed to initialize libcurl globally." << '\n';
      std::cerr << "libcurl error: " << curl_easy_strerror(global_init_res) << '\n';
      std::cerr << "The application cannot continue. Please check your libcurl installation." << '\n';
      printSeparator('!');
      return 1; // Exit if libcurl cannot be initialized
  }
  std::cout << "Network components initialized successfully." << '\n';

  CurlDownloader downloader;
  std::cout << "GitHub API Downloader instance created." << "\n";
  int page {1};
  bool running = true;

  while(running) {
    

    while(true) {
      std::string mode {};
      printSubHeader("Options: \"download\", \"search\", \"exit\",");
      printSubHeader("\"pp (previous page)\", \"at (auth token), \"np (next page).\"");
      std::cout << '\n';
      printSeparator();
      std::cout << " > ";
      std::getline(std::cin, mode);
      

      if (mode == "download" )
      {
        if (found_projects.empty()) { 
          printSubHeader("Please search something first...");
          std::cout << '\n';
          printSeparator();
          continue; 
        }
        int download_int {};
        
        std::cout << " > Project #: ";
        std::cin >> download_int;
        --download_int;

        if (download_int < found_projects.size()) {
          downloader.download_url(found_projects[download_int].html_url, found_projects[download_int].name);
          printSeparator();
          clearInputBuffer();
        } else if (download_int < 0) {
          printSubHeader("Input Error");
          std::cout << "- You must enter an integer larger than 0. \n";
          printSeparator();
          clearInputBuffer();
        } else {
          std::cerr << "Did you forget to search first?\n";
          printSeparator();
          clearInputBuffer();
        }
      } else if (mode == "search") {
        page = 1;
        break;
      } else if (mode == "at") {
        printSubHeader("Please input your Authorization Token");
        downloader.set_auth_token();
      } else if (mode == "exit") { 
        exit(0);
      } else if (mode == "np" || mode == "pp") {
        ++page; 
        if (mode == "pp") { page -= 2; } // different approach
        found_projects.clear();

        long http_status = downloader.searchRepositories(searchTerm, qualifiers, found_projects, page);
        printSubHeader("Search Results");
        
        if (http_status == 200 && found_projects.empty()) {
          std::cout << "No more results found. You might be at the last page." << std::endl;
        }
        
        if (http_status == 200) { // HTTP OK
          std::cout << "API request successful (HTTP 200 OK)." << "\n";
          if (found_projects.empty()) {
              std::cout << "No repositories found matching your criteria." << "\n";
          } else {
              std::cout << "Found " << found_projects.size() << " repository/repositories." << "\n";
              printSeparator();
              for (size_t i = 0; i < found_projects.size(); ++i) {
                const auto& project = found_projects[i];
                std::cout << "Result " << (i + 1) << " of " << found_projects.size() << ":" << "\n";
                std::cout << "  " << std::left << std::setw(15) << "Name:" << project.name << "\n";
                std::cout << "  " << std::left << std::setw(15) << "URL:" << project.html_url << "\n";
                
                std::string desc = project.description;
                if (desc.empty() || desc == "N/A") {
                    desc = "No description provided.";
                } else if (desc.length() > 100) { // Truncate long descriptions for console
                    desc = desc.substr(0, 97) + "...";
                }
                std::cout << "  " << std::left << std::setw(15) << "Description:" << desc << "\n";
                std::cout << "  " << std::left << std::setw(15) << "Stars:" << project.stargazers_count << "\n";
                std::cout << "  " << std::left << std::setw(15) << "Last Push:" << project.pushed_at << "\n";
                if (i < found_projects.size() - 1) {
                    printSeparator('.');
                }
              }
              printSeparator();
            }
          } 
      }
    }

    printSubHeader("Search Configuration");

    searchTerm = getNonEmptyLine("Enter the primary GitHub repository search term (e.g., 'cpp web server'): ");
    std::string qualifier_input;
    std::cout << "\nEnter search qualifiers one by one (e.g., 'language:C++', 'stars:>500', 'topic:game-engine')." << "\n";
    // std::cout << "Press Enter without typing anything to finish adding qualifiers." << "\n";

    int qualifier_count = 1;
    while (true) {
        std::cout << "Qualifier [" << qualifier_count << "] (or press Enter to finish): ";
        std::getline(std::cin, qualifier_input);
        // Trim whitespace
        qualifier_input.erase(0, qualifier_input.find_first_not_of(" \t\n\r\f\v"));
        qualifier_input.erase(qualifier_input.find_last_not_of(" \t\n\r\f\v") + 1);

        if (qualifier_input.empty()) {
            if (qualifier_count == 1) {
                std::cout << "No qualifiers added." << "\n";
            }
            break;
        }
        qualifiers.push_back(qualifier_input);
        std::cout << "  Added qualifier: \"" << qualifier_input << "\"" << "\n";
        qualifier_count++;
    }

    printSubHeader("Performing Search");
    std::cout << "Searching GitHub for repositories matching: \"" << searchTerm << "\"" << "\n";
    if (!qualifiers.empty()) {
        std::cout << "With the following qualifiers:" << "\n";
        for (const auto& q : qualifiers) {
            std::cout << "  - " << q << "\n";
        }
    }
    std::cout << "Please wait, this may take a moment..." << "\n";

    
    long http_status = downloader.searchRepositories(searchTerm, qualifiers, found_projects, 0);

    printSubHeader("Search Results");

    if (http_status == 200) { // HTTP OK
        std::cout << "API request successful (HTTP 200 OK)." << "\n";
        if (found_projects.empty()) {
            std::cout << "No repositories found matching your criteria." << "\n";
        } else {
            std::cout << "Found " << found_projects.size() << " repository/repositories." << "\n";
            printSeparator();
            std::cout << "PG: (" << page << ")";
            printSeparator();
            for (size_t i = 0; i < found_projects.size(); ++i) {
                const auto& project = found_projects[i];
                std::cout << "Result " << (i + 1) << " of " << found_projects.size() << ":" << "\n";
                std::cout << "  " << std::left << std::setw(15) << "Name:" << project.name << "\n";
                std::cout << "  " << std::left << std::setw(15) << "URL:" << project.html_url << "\n";
                
                std::string desc = project.description;
                if (desc.empty() || desc == "N/A") {
                    desc = "No description provided.";
                } else if (desc.length() > 100) { // Truncate long descriptions for console
                    desc = desc.substr(0, 97) + "...";
                }
                std::cout << "  " << std::left << std::setw(15) << "Description:" << desc << "\n";
                std::cout << "  " << std::left << std::setw(15) << "Stars:" << project.stargazers_count << "\n";
                std::cout << "  " << std::left << std::setw(15) << "Last Push:" << project.pushed_at << "\n";
                if (i < found_projects.size() - 1) {
                    printSeparator('.');
                }
            }
            printSeparator();
        }
    } else if (http_status > 0) { // Other HTTP error codes (e.g., 4xx, 5xx)
        std::cerr << "API Request Error: GitHub API returned HTTP status code: " << http_status << "\n";
        std::cerr << "This could be due to various reasons:" << "\n";
        std::cerr << "  - Invalid search query or qualifiers." << "\n";
        std::cerr << "  - API rate limiting (if you've made too many requests)." << "\n";
        std::cerr << "  - GitHub API changes or temporary unavailability." << "\n";
        std::cerr << "  - Network issues preventing a successful connection to GitHub." << "\n";
        std::cerr << "Please check your search terms. If you are rate-limited, try again later." << "\n";
        std::cerr << "Using a GitHub Personal Access Token can increase rate limits." << "\n";
    } else { // Negative values indicate libcurl internal errors
        std::cerr << "Network/libcurl Error: An internal error occurred during the request." << "\n";
        std::cerr << "This means the request likely didn't reach GitHub servers successfully." << "\n";
        std::cerr << "Possible causes include:" << "\n";
        std::cerr << "  - No internet connection or network configuration issues (DNS, firewall)." << "\n";
        std::cerr << "  - Problems with the libcurl library setup on your system." << "\n";
        std::cerr << "libcurl internal code: " << http_status << "\n";
        if (http_status < 0 && http_status > -CURL_LAST) { // Check if it's a valid CURLcode range
             CURLcode curl_err = static_cast<CURLcode>(-http_status); // Convert back to positive CURLcode
             std::cerr << "libcurl error details: " << curl_easy_strerror(curl_err) << "\n";
        }
    }

  }
  std::cout << "\nShutting down network components (libcurl)..." << "\n";
  curl_global_cleanup();
  std::cout << "Network components shut down." << "\n";

  printHeader("Application Finished");
  std::cout << "Thank you for using the GitHub Repository Search Client!" << "\n";

    return 0;
}
