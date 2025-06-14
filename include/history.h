#pragma once

#include "pch.h"
#include <deque>

void addToHistory(int64_t userId, const std::string& state);

std::string goBack(int64_t userId);

std::string getPreviousState(int64_t userId);
