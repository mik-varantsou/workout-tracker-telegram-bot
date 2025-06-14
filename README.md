# Workout Tracker Telegram Bot

This is a Telegram bot for tracking your workout progress. It is written in C++ using the tgbot-cpp library and SQLite for data storage.

---

## Features

- Track workout sessions
- Manage user profiles
- Custom keyboards for ease of use
- Multilanguage support (planned)

---

## Prerequisites

- C++20 compatible compiler (MSVC, GCC, Clang)
- CMake (version 3.15 or higher)
- Git
- [vcpkg](https://github.com/microsoft/vcpkg) package manager

---

## Setup and Build

### 1. Clone the repository
git clone https://github.com/mik-varantsou/workout-tracker-telegram-bot.git
cd workout-tracker-telegram-bot


2. Install dependencies with vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe install sqlite3 sqlitecpp
.\vcpkg.exe integrate install

3. Configure and build the project
Make sure to replace path\to\vcpkg with the actual path to your vcpkg folder.

cd path\to\your\workout-tracker-telegram-bot
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build .



Configuration
Create a .env file in the root of the project (same level as CMakeLists.txt).
Add your Telegram bot token inside the .env file as follows:

BOT_TOKEN=your_telegram_bot_token_here

Important: Keep your bot token private.



Running the Bot
After successful build, run the executable:

.\tgbot_new.exe
The bot will connect to Telegram using the token from .env and start listening for commands.

Contributing
Feel free to open issues or submit pull requests to improve the bot.
