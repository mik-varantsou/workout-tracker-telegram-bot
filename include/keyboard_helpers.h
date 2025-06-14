#pragma once

#include "pch.h"
#include "history.h"
#include "config.h"
#include "database.h"
#include "logic.h"




TgBot::InlineKeyboardButton::Ptr createBackButton(int64_t userId, const std::string& callbackData);
TgBot::InlineKeyboardMarkup::Ptr createBackKeyboard(int64_t userId);

TgBot::InlineKeyboardMarkup::Ptr mergeKeyboards(int64_t userId, const std::string& data, const TgBot::InlineKeyboardMarkup::Ptr& existingKeyboard);

TgBot::InlineKeyboardMarkup::Ptr mergeKeyboardsWithExerciseData(int64_t userId, const TgBot::InlineKeyboardMarkup::Ptr& existingKeyboard, const std::string& exerciseName);

void addButtonsInRows(TgBot::InlineKeyboardMarkup::Ptr& keyboard, const std::vector<TgBot::InlineKeyboardButton::Ptr>& buttons);
void processState(int64_t userId, int32_t messageId, const std::string& state, const std::vector<std::pair<std::string, std::string>>& buttons, bool useDbButtons = false, const std::string& dbPattern = "", bool isExerciseData = false, const std::string& exercise_name = "");

void handleExercise(int64_t userId, int32_t messageId, TgBot::Bot& bot, const std::string& exercise);
void handleProfile(int64_t userId, int32_t messageId, TgBot::Bot& bot);
void handleRecords(int64_t userId, int32_t messageId, TgBot::Bot& bot);

void handleRecordsByCategory(int64_t userId, int32_t messageId, TgBot::Bot& bot, const std::string& category);

