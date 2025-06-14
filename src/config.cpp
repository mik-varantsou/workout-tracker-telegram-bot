#include "config.h"
#include <fstream>
#include <iostream>
#include <filesystem>

std::string readEnvToken(const std::string& filename, const std::string& key) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open .env file: " << filename << std::endl;
        return "";
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(key + "=") == 0) {
            return line.substr(key.size() + 1);
        }
    }
    return "";
}

std::string token = readEnvToken(".env", "BOT_TOKEN");
TgBot::Bot bot(token);
std::string db_path = (std::filesystem::current_path() / "gym.db3").string();
SQLite::Database db(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
