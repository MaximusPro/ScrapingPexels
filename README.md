# ScrapingPexels

A tool for downloading photos and videos from Pexels.com  
(currently — an experimental C++ version)

<p align="center">
  <img src="https://images.pexels.com/photos/1525041/pexels-photo-1525041.jpeg?auto=compress&cs=tinysrgb&w=1260&h=750&dpr=2" width="640" alt="Pexels example photo"/>
</p>

## About the Project

**ScrapingPexels** is a program for bulk downloading free photos and videos from [Pexels.com](https://www.pexels.com/) with API.

## Current Status (February 2026)

- Only a Visual Studio solution (.sln) exists
- Basic project structure is set up
- **No** working page parser or media downloader yet
- No support for keyword search, categories, or curated collections
- No pagination handling, rate limiting, proxies, or delays

## Planned Features

- Keyword / phrase search
- Downloading photos in selected resolution (original / medium / small)
- Video downloading (different qualities)
- Filters: orientation, size, color, minimum likes
- Proxy support and User-Agent rotation
- Saving metadata (author, tags, description, original link)
- Smart delays and speed limiting to avoid bans
- Ability to resume interrupted downloads

## Requirements

- Windows (currently only MSVC project)
- Visual Studio 2022 (or newer)
- C++17 or later
- Likely libraries (to be confirmed):
  - libcurl (HTTP requests)
  - RapidJSON (if JSON parsing is needed)
  - gumbo-parser (HTML parsing)

(exact dependency list will be added after the first working version)

## Build Instructions
1. Clone the repository

```bash
git clone https://github.com/MaximusPro/ScrapingPexels.git
cd ScrapingPexels
```
2. Open Scraper.sln in Visual Studio
3. Select Release x64 configuration (or Debug)
4. Edit file Scraper.cpp find ``string APIKey `` and ``string query``, add API key and searching words
   Example:
   ```C++
 string APIKey = "sgfjdgOPPJIOIJJIjlkmlkp";
 string qurey = "cat";
 ```
5. Build the solution (Build → Build Solution)

More detailed instructions will be added once a working version appears.
# Usage (planned syntax — will be available after v0.1)
```bash
# Example
Scraper.exe --query "japanese cherry blossom" --count 50 --min-likes 200 --output ./photos/
```
# Important Notice

Pexels may block your IP if you scrape too aggressively
Respect the authors and Pexels license terms (CC0 / Public Domain)
Do not use this for large-scale commercial data collection without permission

# License
MIT License (LICENSE file will be added after the first proper release)
Author
MaximusPro
GitHub: @MaximusPro

Contributions are welcome!
Especially helpful would be assistance with:

solid HTTP client implementation
reliable HTML parsing
handling bot protection / Cloudflare (if it appears)
