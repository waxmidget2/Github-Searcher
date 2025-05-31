# GitHub Searcher CLI

A command-line tool to search GitHub repositories using the GitHub Search API, with support for advanced qualifiers, pagination, and secure token management via `.env` files.

---

## Features

- **Search GitHub repositories** by keyword and qualifiers (e.g., language, stars, topics).
- **Secure authentication** using a GitHub Personal Access Token from a `.env` file or environment variable.
- **Pagination** support for browsing multiple result pages.
- **Download repositories** directly (interactive mode or CLI).
- **Clear, formatted output** for easy reading.
- **Cross-platform** (Linux, macOS, Windows with minor adjustments).

---

## Installation

1. **Clone the repository:**
    ```sh
    git clone https://github.com/waxmidget2/github-searcher.git
    cd github-searcher
    ```

2. **Build the project:**
    ```sh
    mkdir build && cd build
    cmake ..
    make
    ```

3. **(Optional) Set up your `.env` file:**
    ```
    GITHUB_TOKEN=your_personal_access_token_here
    ```

---

## Main Application Examples

- **Interactive search for C++ web servers:**
    1. Run the program with no arguments:
        ```sh
        ./github-searcher
        ```
    2. Follow the prompts:
        ```
        --- Options: "download", "search", "exit",
        --- "pp (previous page)", "at (auth token), "np (next page)."
        >
        ```
    3. Type `search` and enter your search term, e.g.:
        ```
        Enter the primary GitHub repository search term (e.g., 'cpp web server'): cpp web server
        Qualifier [1] (or press Enter to finish): language:C++
        Qualifier [2] (or press Enter to finish): stars:>500
        Qualifier [3] (or press Enter to finish):
        ```
    4. View results and use `download` or pagination commands as needed.

- **Download a repository after searching (interactive mode):**
    1. Run the program:
        ```sh
        ./github-searcher
        ```
    2. Perform your search as prompted.
    3. When results are shown, type `download` at the prompt.
    4. Enter the project number you wish to download, for example:
        ```
        > Project #: 2
        ```
    5. The repository will be downloaded to your current directory.

---

## CLI Usage

You can also use the CLI mode for scripting or quick searches:

```sh
./github-searcher -s "cpp web server" -q "stars:>500" -q "language:C++" -p 1
```

**Arguments:**
- `-s`, `--search` : The main search term (required)
- `-q`             : Add a search qualifier (can be repeated)
- `-p`, `--page`   : Page number (optional, default: 1)
- `-d`             : Download the Nth result automatically (optional)
- `-h`, `--help`   : Show help

---

## More CLI Examples

- **Search for C++ projects with more than 1000 stars:**
    ```sh
    ./github-searcher -s "C++" -q "stars:>1000"
    ```

- **Search for Python web frameworks with a specific topic:**
    ```sh
    ./github-searcher -s "web framework" -q "language:Python" -q "topic:web"
    ```

- **Get the second page of results for JavaScript projects:**
    ```sh
    ./github-searcher -s "JavaScript" -p 2
    ```

- **Search for repositories with a specific license:**
    ```sh
    ./github-searcher -s "machine learning" -q "license:mit"
    ```

- **Download the first result for a Rust project directly:**
    ```sh
    ./github-searcher -s "Rust" -q "stars:>500" -d 1
    ```

- **Show help:**
    ```sh
    ./github-searcher --help
    ```

---

## Example Output

```
================================================================================
=== GitHub Repository Search CLI ===
================================================================================
GitHub API token loaded from environment.
Found 5 repositories.
--------------------------------------------------------------------------------
Result 1:
  Name:        scikit-learn/scikit-learn
  URL:         https://github.com/scikit-learn/scikit-learn
  Description: scikit-learn: machine learning in Python
  Stars:       57000
  Last Push:   2024-05-30T12:34:56Z
  License:     BSD-3-Clause
--------------------------------------------------------------------------------
Result 2:
  Name:        keras-team/keras
  URL:         https://github.com/keras-team/keras
  Description: Deep Learning for humans
  Stars:       60000
  Last Push:   2024-05-29T09:12:34Z
  License:     MIT
--------------------------------------------------------------------------------
...
```

---

## Usefulness

- **Developers** can quickly discover trending or relevant repositories for their stack.
- **Researchers** can filter projects by stars, language, or topic for analysis.
- **Students** can find open-source projects to contribute to, filtered by language or popularity.
- **Automated scripts** can use this CLI for batch searching or integration with other tools.

---

## Security

- **No hardcoded credentials:** Tokens are loaded from `.env` or environment variables.
- **Safe token management:** The tool can update your `.env` file with a new token interactively.
- **.env best practices:** Always add `.env` to your `.gitignore`.

---

## Advanced Usage

- **Pagination:** Use `-p 2` to get the second page of results.
- **Multiple qualifiers:** Use `-q` multiple times for advanced filtering.
- **Interactive download:** In interactive mode, you can download repositories directly.

---

## Troubleshooting

- **Rate limiting:** If you see API errors, set a GitHub token in your `.env` to increase your rate limit.
- **Network errors:** Ensure you have internet access and that `libcurl` is installed.

---

## License

MIT License

---

## Contributing

Pull requests and issues are welcome!

---

## Author

[Your Name]  
[Your GitHub Profile]
