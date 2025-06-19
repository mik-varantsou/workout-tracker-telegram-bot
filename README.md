# Workout Tracker Telegram Bot

This is a Telegram bot for tracking your workout progress. It is written in C++ using the [tgbot-cpp](https://github.com/reo7sp/tgbot-cpp) library and SQLite for data storage.

---

## Features

- Track workout sessions  
- Manage user profiles  
- Custom keyboards for ease of use  
- Multilanguage support (planned)  

---

## Prerequisites

- C++20 compiler (MSVC, GCC, Clang)  
- CMake 3.15+  
- Git  
- [vcpkg](https://github.com/microsoft/vcpkg) package manager  

---

## Setup and Build

### 1. Clone the repository

```bash
git clone https://github.com/mik-varantsou/workout-tracker-telegram-bot.git
cd workout-tracker-telegram-bot
```

### 2. Install dependencies with vcpkg
If you don’t have vcpkg installed yet, clone and bootstrap it first:
```bash
git clone https://github.com/microsoft/vcpkg.git

cd vcpkg

.\bootstrap-vcpkg.bat
```

Then install the required libraries:
```bash
.\vcpkg.exe install sqlitecpp zlib openssl curl boost-system boost-filesystem boost-property-tree boost-lexical-cast boost-asio
```

Integrate vcpkg with your system (optional but recommended):

```bash
.\vcpkg.exe integrate install
```


### 3. Configure and build the project
Make a build directory and generate build files with CMake, specifying the vcpkg toolchain file. Replace path\to\vcpkg with your actual vcpkg folder path:

```bash
mkdir build

cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=path\to\vcpkg\scripts\buildsystems\vcpkg.cmake

cmake --build .
```
This will compile the project and produce an executable.
---

## Configuration
To run the bot, you need a Telegram bot token:
1. Create a .env file in the project root (same folder as tgbot_new.exe)
2. Add your token inside .env like this:
   ```ini
   BOT_TOKEN=your_telegram_bot_token_here
   ```
Keep this token secret — do not share it publicly.


---


## Running the Bot
After building, run the executable from the build folder:
```bash
.\tgbot_new.exe
```

---

## Contribution and Support
If you find bugs, have ideas, or want to contribute code — feel free to open issues or submit pull requests.

If you have any questions or need help, just ask!
Good luck and enjoy tracking your workouts! 💪
