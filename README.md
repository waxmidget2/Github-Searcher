# GitHub Searcher CLI

A command-line tool to search GitHub repositories using the GitHub Search API, with support for advanced qualifiers, pagination, and secure token management via `.env` files.

---

## Features

- **Search GitHub repositories** by keyword and qualifiers (e.g., language, stars, topics).
- **Secure authentication** using a GitHub Personal Access Token from a `.env` file or environment variable.
- **Pagination** support for browsing multiple result pages.
- **Download repositories** directly (interactive mode).
- **Clear, formatted output** for easy reading.
- **Cross-platform** (Linux, macOS, Windows with minor adjustments).

---

## Installation

1. **Clone the repository:**
    ```sh
    git clone https://github.com/yourusername/github-searcher.git
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

## Usage

### CLI Mode

```sh
./github-searcher -s "cpp web server" -q "stars:>500" -q "language:C++" -p 1
```

**Arguments:**
- `-s`, `--search` : The main search term (required)
- `-q`             : Add a search qualifier (can be repeated)
- `-p`, `--page`   : Page number (optional, default: 1)
- `-h`, `--help`   : Show help

**Example:**
```sh
./github-searcher -s "machine learning" -q "language:Python" -q "stars:>1000"
```

---

### Example Output

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

Carter Nolan
https://github.com/waxmidget2
