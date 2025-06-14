#pragma once

#include "pch.h"

// Пути и имена
#define DATABASE_PATH "gym.db3"

// Функции
std::string readEnvToken(const std::string& filename, const std::string& key);

// Экспорт переменных
extern TgBot::Bot bot;
extern std::string db_path;
extern SQLite::Database db;