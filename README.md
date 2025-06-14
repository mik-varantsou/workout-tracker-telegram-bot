# Workout Tracker Telegram Bot

A Telegram bot to track workouts and exercise progress.

## Overview

This bot allows users to log workout results, view their profile, and manage data through an easy-to-use Telegram interface.

---

## Requirements

- Windows
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
