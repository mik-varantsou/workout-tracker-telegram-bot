#pragma once

#include "pch.h"
#include "history.h"
#include "config.h"
#include "keyboard_helpers.h"



void handleCustomUserSection(const std::string& callback, int64_t userId, int messageId, SQLite::Database& db, bool isExerciseData);


void onUserMessage(int64_t userId);
void onUserClick(int64_t userId);

std::string getTranslation(const std::string& key, const std::string& language);

TgBot::InlineKeyboardButton::Ptr makeButton(const std::string& key, const std::string& callbackData, const std::string& lang);