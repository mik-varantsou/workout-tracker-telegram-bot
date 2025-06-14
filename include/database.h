#pragma once

#include "pch.h"
#include "history.h"
#include "config.h"
#include "keyboard_helpers.h"




void createDatabase(const std::string& db_path); // creating a database if it does not already exist

bool saveUserButton(int64_t userId, const std::string& buttonName, const std::string& callbackData, SQLite::Database &db);
bool deleteUserButton(int64_t userId, const std::string& callbackData, SQLite::Database& db);

bool saveUserResults(int64_t userId, SQLite::Database& db, std::string weight, std::string reps, std::string date, std::string buttonName, std::string callback_data);
bool viewUserResults(int64_t userId, SQLite::Database& db, const std::string& buttonName);
bool deleteUserResults(int64_t userId, SQLite::Database& db, const std::string& callbackData);

bool isCustomUserSection(const std::string& callback, SQLite::Database& db, int64_t userId);


std::string getSingleStringValue(SQLite::Database& db, const std::string& query, int64_t userId, const std::string& secondParam = "");
std::string getButtonPressed(int64_t userId, SQLite::Database& db);
std::string getCustomButtonName(int64_t userId, SQLite::Database& db, const std::string& callback_data);
std::string getState(int64_t userId, SQLite::Database& db);
std::string getLanguage(int64_t userId, SQLite::Database& db);

void setUserField(int64_t userId, SQLite::Database& db, const std::string& fieldName, const std::string& value);
void setState(int64_t userId, SQLite::Database& db, const std::string& state);
void setExercise(int64_t userId, SQLite::Database& db, const std::string& exercise);
void setButtonPressed(int64_t userId, SQLite::Database& db, const std::string& callback_data);
void setLanguage(int64_t userId, SQLite::Database& db, const std::string& language);




