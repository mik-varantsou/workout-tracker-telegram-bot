#include "history.h"

const size_t MAX_HISTORY_SIZE = 20;
std::unordered_map<int64_t, std::deque<std::string>> userHistory;

void addToHistory(int64_t userId, const std::string& state) {
    auto& history = userHistory[userId];

    if (!history.empty() && history.back() == state) {
        return;
    }

    history.push_back(state);
    std::cout << "addToHistory: " << state << std::endl;

    if (history.size() > MAX_HISTORY_SIZE) {
        history.pop_front();
    }

    // Выводим всю историю для отладки
    std::cout << "Текущая история для " << userId << ": ";
    for (const auto& s : history) {
        std::cout << s << " -> ";
    }
    std::cout << std::endl;
}

std::string goBack(int64_t userId) {
    auto& history = userHistory[userId];

    std::cout << "[Перед goBack] История для " << userId << ": ";
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << history[i];
        if (i + 1 < history.size()) std::cout << " -> ";
    }
    std::cout << std::endl;

    if (history.empty()) {
        std::cout << "История пуста! Возвращаем menu." << std::endl;
        return "menu_back";
    }

    if (history.size() > 1) {
        history.pop_back();  // Удаляем последний элемент
        std::string newState = history.back();
        std::cout << "Переход назад: " << newState << std::endl;
        return newState;
    }

    std::cout << "История содержит один элемент! Возвращаем menu." << std::endl;
    return "menu_back";
}

std::string getPreviousState(int64_t userId) {
    auto& history = userHistory[userId];
    if (history.size() > 1) {
        std::cout << "getPreviousState: возвращаем предпоследнее состояние: " << history[history.size() - 2] << std::endl;
        return history[history.size() - 2];
    }
    std::cout << "getPreviousState: история пуста или один элемент, возвращаем menu_back" << std::endl;
    return "menu_back";
}
