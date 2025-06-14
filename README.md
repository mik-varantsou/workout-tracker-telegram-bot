# Workout Tracker Telegram Bot

A Telegram bot to track workouts and exercise progress.

## Overview

This bot allows users to log workout results, view their profile, and manage data through an easy-to-use Telegram interface.

---

## Requirements

- Windows / Linux
- C++20 compatible compiler
- [CMake](https://cmake.org/) 3.15 or higher
- [vcpkg](https://github.com/microsoft/vcpkg) (recommended) to manage dependencies
- Libraries:
  - tgbot-cpp (included as a submodule under `external/`)
  - SQLiteCpp
  - SQLite3

---

## Installing dependencies with vcpkg (recommended)

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
.\bootstrap-vcpkg.bat # Windows

./vcpkg install sqlite3 sqlitecpp
Configuration
Create a .env file in the root of the project with the following content:

BOT_TOKEN=YOUR_TELEGRAM_BOT_TOKEN
Building the project
Clone this repository:


git clone https://github.com/mik-varantsou/workout-tracker-telegram-bot.git
cd workout-tracker-telegram-bot
Create a build directory and navigate to it:

mkdir build
cd build
Run CMake and build the project:

cmake .. -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
Replace path_to_vcpkg with the actual path to your vcpkg installation.

Running the bot
Run the compiled executable:

bash
Copy
Edit
./tgbot_new   # or tgbot_new.exe on Windows
Make sure the .env file with your bot token is located alongside the executable.

License
This project is licensed under the MIT License.

Contact
If you have any questions or suggestions, please open an issue or contact me directly.

