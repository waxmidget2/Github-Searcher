#include "curl_downloader.h"
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <git2.h>
#include <fstream>

CurlDownloader::CurlDownloader() {
  curl_handle = curl_easy_init();
  if (!curl_handle) { std::cerr << "Error: curl_easy_init() failed. CurlDownloader will not work." << "\n"; }
  git_libgit2_init();
}

CurlDownloader::~CurlDownloader() {
  if (curl_handle) { curl_easy_cleanup(curl_handle); }
  git_libgit2_shutdown();
}
// standard doc function (deprecated currently)
// size_t CurlDownloader::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
//  return fwrite(contents, size, nmemb, (FILE*)userp);
// }

// modified documentation function
size_t CurlDownloader::write_callback_std_string(void* contents, size_t size, size_t nmemb, std::string* s) {
  size_t new_length = size * nmemb;
  
  try { s->append(static_cast<char*>(contents), new_length); } 
  
  catch (const std::bad_alloc& e) 
  {
      std::cerr << "Error: Failed to allocate memory in write_callback_std_string: " << e.what() << "\n";
      return 0;
  }
  return new_length;
}
void CurlDownloader::set_auth_token() {
    std::string token;
    std::cout << "Enter GitHub API token: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clear buffer
    std::getline(std::cin, token);
    this->auth_token = token;

    // Save token to .env file
    std::ofstream envFile(".env");
    if (envFile.is_open()) {
        envFile << "GITHUB_TOKEN=" << token << std::endl;
        envFile.close();
        std::cout << "Token saved to .env file." << std::endl;
    } else {
        std::cerr << "Warning: Could not write to .env file." << std::endl;
    }

    if(auth_token.empty()) {
        std::cerr << "Warning: Authorization token is empty. This may limit API access." << "\n";
    } else {
        std::cout << "Authorization token set successfully." << "\n";
    }
}

void CurlDownloader::set_auth_token(const std::string& token) {
    this->auth_token = token;
    if(auth_token.empty()) {
        std::cerr << "Warning: Authorization token is empty. This may limit API access." << "\n";
    } else {
        std::cout << "Authorization token set successfully." << "\n";
    }
}

std::string CurlDownloader::urlEncode(const std::string& str_to_encode) {
  if (!curl_handle) {
      std::cerr << "Error: curl_handle not initialized in urlEncode." << "\n";
      return "";
  }
  char* encoded_output = curl_easy_escape(curl_handle, str_to_encode.c_str(), static_cast<int>(str_to_encode.length()));
  if (encoded_output) {
      std::string result(encoded_output);
      curl_free(encoded_output);
      return result;
  }
  std::cerr << "Error: curl_easy_escape failed for string: " << str_to_encode << "\n";
  return "";
}

long CurlDownloader::searchRepositories(const std::string& search_term,
                                      const std::vector<std::string>& qualifiers,
                                      std::vector<ProjectInfo>& projects_out,
                                      int page) {
  // previous projects get tossed out
  projects_out.clear();
  
  // danger check
  if (!curl_handle) {
      std::cerr << "Error: CurlDownloader not properly initialized (curl_handle is null)." << "\n";
      return -1000;
  }
  
  // initialize variables
  std::string read_buffer;
  long http_code = 0;
  CURLcode res;

  // Process qualifiers into url format
  std::string query_components = urlEncode(search_term);
  for (const std::string& qualifier : qualifiers) {
      if (!query_components.empty() && !qualifier.empty()) {
          query_components += "+";
      }
      query_components += urlEncode(qualifier); // add the url standard encoding to them
  }

  // build url
  std::string full_api_url = "https://api.github.com/search/repositories?q=" + query_components;
  full_api_url += "&per_page=5";

  if (page > 1) {
      full_api_url += "&page=" + std::to_string(page);
  }

  std::cout << "CurlDownloader: Making API request to: " << full_api_url << "\n";
  // curl parameters, pretty straight forward in the libcurl doc
  
  curl_easy_setopt(curl_handle, CURLOPT_URL, full_api_url.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "MyGitHubClient/1.0");
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, CurlDownloader::write_callback_std_string);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &read_buffer);
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);

  struct curl_slist* headers = nullptr;

  if(!auth_token.empty()) {
    std::string auth_header = "Authorization: Bearer " + auth_token;
    headers = curl_slist_append(headers, auth_header.c_str());
  }

  headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
  
  curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

  res = curl_easy_perform(curl_handle);
  curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

  if (headers) {
      curl_slist_free_all(headers);
  }

  if (res != CURLE_OK) {
      std::cerr << "Error: curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
      return (http_code == 0) ? -static_cast<long>(res) : http_code;
  }

  std::cout << "CurlDownloader: Received HTTP Status Code: " << http_code << "\n";
  if (http_code == 200) {
      try {
          nlohmann::json json_response = nlohmann::json::parse(read_buffer);

          if (json_response.contains("items") && json_response["items"].is_array()) {
              for (const auto& item : json_response["items"]) {
                  ProjectInfo project;
                  project.name = item.value("full_name", "N/A");
                  project.html_url = item.value("html_url", "N/A");
                  if (item.contains("description") && !item["description"].is_null()) {
                      project.description = item["description"].get<std::string>();
                  } else {
                      project.description = "N/A";
                  }
                  project.pushed_at = item.value("pushed_at", "N/A");
                  project.stargazers_count = item.value("stargazers_count", 0);
                  // Add license parsing
                  if (item.contains("license") && item["license"].is_object() && item["license"].contains("spdx_id")) {
                      project.license = item["license"]["spdx_id"].get<std::string>();
                      if (project.license == "NOASSERTION") project.license = "No license";
                  } else {
                      project.license = "Unknown";
                  }
                  projects_out.push_back(project);
              }
              std::cout << "CurlDownloader: Successfully parsed " << projects_out.size() << " items." << "\n";
          } else {
              std::cerr << "Warning: JSON response does not contain 'items' array or is not structured as expected." << "\n";
              if (json_response.contains("message")) {
                    std::cerr << "GitHub API Message: " << json_response["message"].dump() << "\n";
              }
          }
      } catch (const nlohmann::json::parse_error& e) {
          std::cerr << "Error: JSON parsing failed: " << e.what() << "\n";
          std::cerr << "Received data that caused parsing error: " << read_buffer.substr(0, 500) << "..." << "\n";
      }
  } else {
      std::cerr << "Error: GitHub API request failed with HTTP status: " << http_code << "\n";
      if (!read_buffer.empty()) {
          std::cerr << "Response body from server: " << read_buffer.substr(0, 500) << "..." << "\n";
          try {
                nlohmann::json error_json = nlohmann::json::parse(read_buffer);
                if (error_json.contains("message")) {
                    std::cerr << "GitHub API Error Message: " << error_json["message"].dump() << "\n";
                }
                if (error_json.contains("documentation_url")) {
                    std::cerr << "Documentation URL: " << error_json["documentation_url"].dump() << "\n";
                }
          } catch (const nlohmann::json::parse_error&) {
            std::cerr << "Error: Parsing error prevents JSON dumping.";
          }
      }
  }
  return http_code;
}

void CurlDownloader::download_url(const std::string& url, const std::string& name) {
    if (!curl_handle) {
        std::cerr << "Error: CurlDownloader not properly initialized (curl_handle is null)." << "\n";
        return;
    }

    std::filesystem::path install_dir = "packages";
    if (!std::filesystem::exists(install_dir)) {
        try {
            std::filesystem::create_directories(install_dir);
            std::cout << "Created directory: " << install_dir << std::endl;
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error creating directory '" << install_dir << "': " << e.what() << std::endl;
            return;
        }
    }

    std::string filename = name; 

    size_t last_slash = url.find_last_of('/');
    std::string potential_filename_in_url = (last_slash != std::string::npos) ? url.substr(last_slash + 1) : url;
    size_t last_dot = potential_filename_in_url.find_last_of('.');

    std::replace_if(filename.begin(), filename.end(), [](char c) {
        return (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
                c == '"' || c == '<' || c == '>' || c == '|' || c < 32);
    }, '_');

    std::filesystem::path output_path = install_dir / filename;

    if (!std::filesystem::exists(output_path.parent_path())) {
        try {
            std::filesystem::create_directories(output_path.parent_path());
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error creating parent directory for clone target '" << output_path.parent_path() << "': " << e.what() << std::endl;
            return;
        }
    }

    git_repository* repo = nullptr;
    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;

    std::string git_clone_url = url; 

    int git_clone_res = git_clone(&repo, git_clone_url.c_str(), output_path.string().c_str(), &clone_opts);

    if (git_clone_res != 0) {
        const git_error* err = git_error_last();
        std::cerr << "Error: git_clone failed for URL '" << git_clone_url << "' to '" << output_path << "': "
                  << (err ? err->message : "Unknown error") << std::endl;
    } else {
        std::cout << "Successfully cloned repository '" << git_clone_url << "' to: " << output_path << std::endl;
    }

    if (repo) {
        git_repository_free(repo);
    }

    curl_easy_reset(curl_handle);

    std::string owner_repo_part;
    size_t github_pos = url.find("github.com/");
    if (github_pos != std::string::npos) {
        owner_repo_part = url.substr(github_pos + 11); // "owner/repo.git" or "owner/repo"
        if (owner_repo_part.length() > 4 && owner_repo_part.substr(owner_repo_part.length() - 4) == ".git") {
            owner_repo_part = owner_repo_part.substr(0, owner_repo_part.length() - 4);
        }
    } else {
        std::cerr << "Warning: Could not parse GitHub owner/repo from URL for ZIP download: " << url << std::endl;
        return;
    }
    return; 
}

