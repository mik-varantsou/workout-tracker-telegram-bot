#include "keyboard_helpers.h"






TgBot::InlineKeyboardButton::Ptr createBackButton(int64_t userId, const std::string& callbackData) {
    auto back = std::make_shared<TgBot::InlineKeyboardButton>();
    const std::string lang = getLanguage(userId, db);
    back->text = getTranslation("back", lang);
    back->callbackData = "back_btn:" + callbackData;
    return back;
}
TgBot::InlineKeyboardMarkup::Ptr createBackKeyboard(int64_t userId) {
    const std::string prevState = getPreviousState(userId); 
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    keyboard->inlineKeyboard.push_back({ createBackButton(userId, prevState) });
    return keyboard;
}
TgBot::InlineKeyboardMarkup::Ptr mergeKeyboards(int64_t userId, const std::string& data, const TgBot::InlineKeyboardMarkup::Ptr& existingKeyboard) {
    try {
        // Получаем текущее состояние пользователя
        SQLite::Statement getStateQuery(db, "SELECT state FROM users WHERE user_id = ?");
        getStateQuery.bind(1, userId);

        std::string currentState;
        if (getStateQuery.executeStep()) {
            currentState = getStateQuery.getColumn(0).getText();
        }

        std::cout << "mergeKeyboards: " << data << std::endl;

        // Запрашиваем кастомные кнопки
        SQLite::Statement query(db, "SELECT button_name, callback_data FROM custom_buttons WHERE user_id = ? AND callback_data LIKE ?");
        query.bind(1, userId);
        query.bind(2, data + "%");

        // Создаём клавиатуру, копируя существующую (если есть)
        auto mergedKeyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
        if (existingKeyboard && !existingKeyboard->inlineKeyboard.empty()) {
            mergedKeyboard->inlineKeyboard = existingKeyboard->inlineKeyboard;
        }

        // Создаём новые кнопки по 2 в ряд
        std::vector<TgBot::InlineKeyboardButton::Ptr> row;
        while (query.executeStep()) {
            auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
            btn->text = query.getColumn(0).getText();

            if (currentState == "deleting")
                btn->callbackData = "delete_end:" + std::string(query.getColumn(1).getText());
            else
                btn->callbackData = query.getColumn(1).getText();

            std::cout << "Добавляем кнопку: " << btn->text << ", " << btn->callbackData << std::endl;

            row.push_back(btn);
            if (row.size() == 2) {
                mergedKeyboard->inlineKeyboard.push_back(row);
                row.clear();
            }
        }

        if (!row.empty()) {
            mergedKeyboard->inlineKeyboard.push_back(row);
        }

        std::cout << "Количество кнопок в клавиатуре: " << mergedKeyboard->inlineKeyboard.size() << std::endl;
        return mergedKeyboard;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при работе с базой данных: " << e.what() << std::endl;
        return nullptr;
    }
}
TgBot::InlineKeyboardMarkup::Ptr mergeKeyboardsWithExerciseData(int64_t userId, const TgBot::InlineKeyboardMarkup::Ptr& existingKeyboard, const std::string& exerciseName) {
    try {
        SQLite::Statement query(
            db,
            "SELECT exercise_name, weight, reps, training_date, callback_data "
            "FROM user_trainings "
            "WHERE user_id = ? AND exercise_name = ? "
            "ORDER BY training_date DESC LIMIT 10"
        );
        query.bind(1, userId);
        query.bind(2, exerciseName);

        auto mergedKeyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();

        // Добавляем существующие кнопки, если есть
        if (existingKeyboard && !existingKeyboard->inlineKeyboard.empty()) {
            mergedKeyboard->inlineKeyboard = existingKeyboard->inlineKeyboard;
        }

        std::vector<TgBot::InlineKeyboardButton::Ptr> row;

        while (query.executeStep()) {
            std::string name = query.getColumn(0).getText();
            int weight = query.getColumn(1).getInt();
            int reps = query.getColumn(2).getInt();
            std::string date = query.getColumn(3).getText();
            std::string callback = query.getColumn(4).getText();

            std::stringstream buttonText;
            buttonText << name << ": " << weight << " кг × " << reps << " (" << date << ")";

            auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
            btn->text = buttonText.str();
            btn->callbackData = "custom_delete_end:" + callback;

            std::cout << "Добавляем кнопку: " << btn->text << ", " << btn->callbackData << std::endl;

            row.push_back(btn);
            if (row.size() == 2) {
                mergedKeyboard->inlineKeyboard.push_back(row);
                row.clear();
            }
        }

        if (!row.empty()) {
            mergedKeyboard->inlineKeyboard.push_back(row);
        }

        return mergedKeyboard;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при работе с базой данных: " << e.what() << std::endl;
        return nullptr;
    }
}
void addButtonsInRows(TgBot::InlineKeyboardMarkup::Ptr& keyboard, const std::vector<TgBot::InlineKeyboardButton::Ptr>& buttons) {
    std::vector<TgBot::InlineKeyboardButton::Ptr> row;
    for (const auto& btn : buttons) {
        row.push_back(btn);
        if (row.size() == 2) {
            keyboard->inlineKeyboard.push_back(row);
            row.clear();
        }
    }
    if (!row.empty()) {
        keyboard->inlineKeyboard.push_back(row);
    }
}
void processState(int64_t userId, int32_t messageId, const std::string& state, const std::vector<std::pair<std::string, std::string>>& buttons, bool useDbButtons, const std::string& dbPattern, bool isExerciseData, const std::string& exercise_name) {
    addToHistory(userId, state);

    // Создаём начальную клавиатуру и добавляем кнопки вручную
    auto baseKeyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    std::vector<TgBot::InlineKeyboardButton::Ptr> staticButtons;

    for (const auto& btn : buttons) {
        auto button = std::make_shared<TgBot::InlineKeyboardButton>();
        button->text = btn.first;
        button->callbackData = btn.second;
        staticButtons.push_back(button);
    }

    addButtonsInRows(baseKeyboard, staticButtons);

    // Добавление кастомных или упражненийных кнопок из БД
    TgBot::InlineKeyboardMarkup::Ptr finalKeyboard = baseKeyboard;

    if (useDbButtons && !dbPattern.empty()) {
        if (isExerciseData) {
            auto dbKeyboard = mergeKeyboardsWithExerciseData(userId, finalKeyboard, exercise_name);
            if (dbKeyboard) finalKeyboard = dbKeyboard;
        }
        else {
            auto dbKeyboard = mergeKeyboards(userId, dbPattern, finalKeyboard);
            if (dbKeyboard) finalKeyboard = dbKeyboard;
        }
    }

    // Добавляем кнопку "Назад"
    finalKeyboard->inlineKeyboard.push_back({ createBackButton(userId, getPreviousState(userId)) });

    // Отправка клавиатуры
    try {
        bot.getApi().editMessageText(
            getTranslation("select", getLanguage(userId, db)),
            userId,
            messageId,
            "",
            "",
            nullptr,
            finalKeyboard
        );
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при отправке сообщения: " << e.what() << std::endl;
    }
}
void handleExercise(int64_t userId, int32_t messageId, TgBot::Bot& bot, const std::string& exercise) {
    addToHistory(userId, exercise);
    setState(userId, db, "resting");
    setExercise(userId, db, exercise);
    std::string language = getLanguage(userId, db);

    std::string prefix = "btn_" + exercise + "%";

    // Подготавливаем фиксированные кнопки
    std::vector<std::pair<std::string, std::string>> fixedButtons;

    try {
        SQLite::Statement query(db, "SELECT COUNT(*) FROM custom_buttons WHERE user_id = ? AND callback_data LIKE ?");
        query.bind(1, userId);
        query.bind(2, prefix);

        if (query.executeStep()) {
            int buttonCount = query.getColumn(0).getInt();
            std::cout << "HandleExercise: found " << buttonCount << " custom buttons\n";

            if (buttonCount < 8) {
                fixedButtons.emplace_back(getTranslation("add_exercise", language), "add:" + exercise);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при подсчёте кастомных кнопок: " << e.what() << std::endl;
    }

    fixedButtons.emplace_back(getTranslation("delete_exercise", language), "delete_start:" + exercise);

    // Создаём клавиатуру с фиксированными кнопками
    auto baseKeyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    std::vector<TgBot::InlineKeyboardButton::Ptr> row;
    for (auto& [text, cb] : fixedButtons) {
        auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
        btn->text = text;
        btn->callbackData = cb;
        row.push_back(btn);
        if (row.size() == 2) {
            baseKeyboard->inlineKeyboard.push_back(row);
            row.clear();
        }
    }
    if (!row.empty()) {
        baseKeyboard->inlineKeyboard.push_back(row);
    }

    // Получаем объединённую клавиатуру с кастомными кнопками
    TgBot::InlineKeyboardMarkup::Ptr fullKeyboard = mergeKeyboards(userId, prefix, baseKeyboard);
    if (!fullKeyboard) {
        fullKeyboard = baseKeyboard; // Fallback
    }

    // Добавляем кнопку назад
    fullKeyboard->inlineKeyboard.push_back({ createBackButton(userId, getPreviousState(userId)) });

    // Отправляем сообщение
    try {
        if (messageId == -1) {
            bot.getApi().sendMessage(userId, getTranslation("menu", language), nullptr, 0, fullKeyboard);
        }
        else {
            bot.getApi().editMessageText(getTranslation("menu", language), userId, messageId, "", "", nullptr, fullKeyboard);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при отправке/редактировании сообщения: " << e.what() << std::endl;
    }
}
void handleProfile(int64_t userId, int32_t messageId, TgBot::Bot& bot) {
    addToHistory(userId, "profile");
    SQLite::Statement query(db, R"(
        SELECT created_at, last_active, message_count, button_count
        FROM user_stats WHERE user_id = ?
    )");
    query.bind(1, userId);
    std::string language = getLanguage(userId, db);

    std::string message;
    if (query.executeStep()) {
        int64_t created = query.getColumn(0).getInt64();
        int64_t last = query.getColumn(1).getInt64();
        int messageCount = query.getColumn(2).getInt();
        int buttonCount = query.getColumn(3).getInt();

        int64_t secondsOnline = last - created;
        int days = secondsOnline / 86400;
        int hours = (secondsOnline % 86400) / 3600;
        int minutes = (secondsOnline % 3600) / 60;

        message = getTranslation("your_profile", language) + "\n";
        message += getTranslation("time_spent", language) + std::to_string(days) + getTranslation("days", language) + std::to_string(hours) + getTranslation("minutes", language) + std::to_string(minutes) + getTranslation("seconds", language)+"\n";
        message += getTranslation("messages", language) + std::to_string(messageCount);
        message += "\n" + getTranslation("buttons", language) + std::to_string(buttonCount);
    }
    else {
        message = getTranslation("no_information", language);
    }

    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    std::vector<TgBot::InlineKeyboardButton::Ptr> row;

    auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
    btn->text = getTranslation("records", language);
    btn->callbackData = "records";
    row.push_back(btn);

    keyboard->inlineKeyboard.push_back(row);
    keyboard->inlineKeyboard.push_back({ createBackButton(userId, "main_menu") });

    if (messageId == -1) {
        bot.getApi().sendMessage(userId, message, nullptr, 0, keyboard, "HTML");
    }
    else {
        bot.getApi().editMessageText(message, userId, messageId, "", "HTML", nullptr, keyboard);
    }
}
void handleRecords(int64_t userId, int32_t messageId, TgBot::Bot& bot) {
    std::string language = getLanguage(userId, db);
    addToHistory(userId, "records");
    std::string message = getTranslation("select_category", language);
    std::map<std::string, std::string> categories = {
        {"chest", getTranslation("chest", language)}, {"spine", getTranslation("spine", language)}, {"abs", getTranslation("abs", language)},
        {"arms", getTranslation("arms", language)}, {"legs", getTranslation("legs", language)}
    };
    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    for (const auto& category : categories) {
        auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
        btn->text = category.second; // Можешь перевести в человекочитаемый вид
        btn->callbackData = "records_cat_" + category.first;
        keyboard->inlineKeyboard.push_back({ btn });
    }
    keyboard->inlineKeyboard.push_back({ createBackButton(userId, "records") });
    bot.getApi().editMessageText(message, userId, messageId, "", "HTML", nullptr, keyboard);
}
void handleRecordsByCategory(int64_t userId, int32_t messageId, TgBot::Bot& bot, const std::string& category) {
    std::string language = getLanguage(userId, db);
    addToHistory(userId, "records_cat_" + category);
    std::map<std::string, std::string> categoryNames = {
        {"chest", getTranslation("chest", language)}, {"spine", getTranslation("spine", language)}, {"abs", getTranslation("abs", language)},
        {"arms", getTranslation("arms", language)}, {"legs", getTranslation("legs", language)}
    };
    std::string title = categoryNames.count(category) ? categoryNames[category] : category;
    std::string prefix = "btn_" + category + "%";
    std::string message = getTranslation("records_by_category", language) + title + "</b>\n";
    bool hasAnyData = false;

    try {
        // Максимальный вес
        SQLite::Statement maxWeightQuery(db, R"(
            SELECT exercise_name, weight, reps, training_date
            FROM user_trainings
            WHERE user_id = ? AND callback_data LIKE ?
            ORDER BY weight DESC
            LIMIT 1
        )");
        maxWeightQuery.bind(1, userId);
        maxWeightQuery.bind(2, prefix);

        if (maxWeightQuery.executeStep()) {
            hasAnyData = true;
            std::string exercise = maxWeightQuery.getColumn(0).getText();
            int weight = maxWeightQuery.getColumn(1).getInt();
            int reps = maxWeightQuery.getColumn(2).getInt();
            std::string date = maxWeightQuery.getColumn(3).getText();

            message += getTranslation("max_weight", language) + std::to_string(weight) + getTranslation("kg", language) + " (" +
                exercise + ", " + std::to_string(reps) + getTranslation("reps", language) + date + ")\n";
        }

        // Максимальные повторения
        SQLite::Statement maxRepsQuery(db, R"(
            SELECT exercise_name, reps, weight, training_date
            FROM user_trainings
            WHERE user_id = ? AND callback_data LIKE ?
            ORDER BY reps DESC
            LIMIT 1
        )");
        maxRepsQuery.bind(1, userId);
        maxRepsQuery.bind(2, prefix);

        if (maxRepsQuery.executeStep()) {
            hasAnyData = true;
            std::string exercise = maxRepsQuery.getColumn(0).getString();
            int reps = maxRepsQuery.getColumn(1).getInt();
            int weight = maxRepsQuery.getColumn(2).getInt();
            std::string date = maxRepsQuery.getColumn(3).getString();

            message += getTranslation("max_reps", language) + std::to_string(reps) + " (" +
                exercise + ", " + std::to_string(weight) + " " + getTranslation("kg", language) + ", " + date + ")";
        }

        if (!hasAnyData) {
            message += getTranslation("no_information_category", language);
        }

    }
    catch (const std::exception& e) {
        message = "❌ Ошибка при получении рекордов: ";
        message += e.what();
    }

    auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
    keyboard->inlineKeyboard.push_back({ createBackButton(userId, "records") });

    if (messageId == -1) {
        bot.getApi().sendMessage(userId, message, nullptr, 0, keyboard, "HTML");
    }
    else {
        bot.getApi().editMessageText(message, userId, messageId, "", "HTML", nullptr, keyboard);
    }
}



