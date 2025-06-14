#include "logic.h"

// поправить
void handleCustomUserSection(const std::string& callback, int64_t userId, int messageId, SQLite::Database& db, bool isExerciseData) {
    if (isExerciseData) {
        // Получить ранее нажатую кнопку (например, btn_chest)
        std::string buttonPressed = getButtonPressed(userId, db);

        // Получаем упражнения пользователя
        SQLite::Statement getExerciseData(db, R"(
            SELECT exercise_name, weight, reps, training_date, callback_data
            FROM user_trainings
            WHERE user_id = ?
            ORDER BY training_date DESC
            LIMIT 10
        )");
        getExerciseData.bind(1, userId);

        while (getExerciseData.executeStep()) {
            std::string exercise = getExerciseData.getColumn(0).getText();
            int weight = getExerciseData.getColumn(1).getInt();
            int reps = getExerciseData.getColumn(2).getInt();
            std::string date = getExerciseData.getColumn(3).getText();
            std::cout << "💪 " << exercise << " | " << weight << getTranslation("kg", getLanguage(userId, db)) << " × " << reps << " (" << date << ")\n";
        }

        // Сохраняем кнопку, которую нажал пользователь
        SQLite::Statement saveBtn(db, "UPDATE users SET button_pressed = ? WHERE user_id = ?");
        saveBtn.bind(1, callback);
        saveBtn.bind(2, userId);
        saveBtn.exec();

        setState(userId, db, "resting");

        // Вызываем состояние с клавиатурой на основе шаблона btn_<callback>
        processState(userId, messageId, callback, {}, true, buttonPressed + "_%", true);
    }
    else {
        std::string buttonName;

        // Находим имя кастомной кнопки
        SQLite::Statement stmt(db, R"(
            SELECT button_name FROM custom_buttons
            WHERE user_id = ? AND callback_data = ?
        )");
        stmt.bind(1, userId);
        stmt.bind(2, callback);

        if (!stmt.executeStep()) {
            bot.getApi().sendMessage(userId, "❌ Ошибка: кнопка не найдена.");
            return;
        }

        buttonName = stmt.getColumn(0).getText();

        // Сохраняем callback как нажатую кнопку
        SQLite::Statement saveBtn(db, "UPDATE users SET button_pressed = ? WHERE user_id = ?");
        saveBtn.bind(1, callback);
        saveBtn.bind(2, userId);
        saveBtn.exec();

        setState(userId, db, "resting");

        // Вызываем processState с действиями для кастомной кнопки
        processState(userId, messageId, callback, {
            {"Добавить упражнение", "add:" + callback},
            {"Удалить упражнение", "delete_start:" + callback}
            }, true, "btn_" + callback + "%");
    }
}

void onUserMessage(int64_t userId) {
    SQLite::Statement insert(db, R"(
        INSERT INTO user_stats (user_id, created_at, last_active, message_count)
        VALUES (?, strftime('%s','now'), strftime('%s','now'), 1)
        ON CONFLICT(user_id) DO UPDATE SET
            last_active = strftime('%s','now'),
            message_count = message_count + 1
    )");
    insert.bind(1, userId);
    insert.exec();
}
void onUserClick(int64_t userId) {
    SQLite::Statement insert(db, R"(
        INSERT INTO user_stats (user_id, last_active, button_count)
        VALUES (?, strftime('%s','now'), 1)
        ON CONFLICT(user_id) DO UPDATE SET
            last_active = strftime('%s','now'),
            button_count = button_count + 1
    )");
    insert.bind(1, userId);
    insert.exec();
}

std::map<std::string, std::map<std::string, std::string>> translations = {
    {"start", {
        {"ru", "Привет! Я — твой персональный бот для записи силовых показателей и отслеживания прогресса в зале.\nЗдесь ты сможешь удобно фиксировать достижения, наблюдать рост результатов и ставить новые цели.\nГотов начать? Поехали!"},
        {"en", "Hi! I'm your personal bot for tracking strength records and gym progress.\nHere, you can easily log achievements, monitor your growth, and set new goals.\nReady to start? Let's go!"},
        {"pl", "Cześć! Jestem Twoim osobistym botem do zapisywania wyników siłowych i śledzenia postępów na siłowni.\nTutaj wygodnie zapiszesz swoje osiągnięcia, będziesz śledzić postępy i wyznaczać nowe cele.\nGotowy? Zaczynamy!"}
    }},

    {"menu", {
        {"ru", "🏠 Главное меню:"},
        {"en", "🏠 Main menu:"},
        {"pl", "🏠 Menu główne:"}
    }},
    {"menu_profile", {
        {"ru", "👤 Профиль"},
        {"en", "👤 Profile"},
        {"pl", "👤 Profil"}
    }},
    {"menu_training", {
        {"ru", "💪 Тренировка"},
        {"en", "💪 Training"},
        {"pl", "💪 Trening"}
    }},

    {"lang_en", {
        {"ru", "🇺🇸 Английский"}, {"en", "🇺🇸 English"}, {"pl", "🇺🇸 Angielski"}
    }},
    {"lang_pl", {
        {"ru", "🇵🇱 Польский"}, {"en", "🇵🇱 Polish"}, {"pl", "🇵🇱 Polski"}
    }},
    {"lang_ru", {
        {"ru", "🇷🇺 Русский"}, {"en", "🇷🇺 Russian"}, {"pl", "🇷🇺 Rosyjski"}
    }},


    {"chest", {
        {"ru", "💪Грудь"}, {"en", "💪Chest"}, {"pl", "💪Klatka piersiowa"}
    }},
    {"spine", {
        {"ru", "🌀Спина"}, {"en", "🌀Back"}, {"pl", "🌀Plecy"}
    }},
    {"arms", {
        {"ru", "🦾Руки"}, {"en", "🦾Arms"}, {"pl", "🦾Ręce"}
    }},
    {"abs", {
        {"ru", "🔥Пресс"}, {"en", "🔥Abs"}, {"pl", "🔥Brzuch"}
    }},
    {"legs", {
        {"ru", "🦵Ноги"}, {"en", "🦵Legs"}, {"pl", "🦵Nogi"}
    }},


    {"add_exercise", {
        {"ru", "➕ Добавить упражнение"}, {"en", "➕ Add exercise"}, {"pl", "➕ Dodać ćwiczenie"}
    }},
    {"delete_exercise", {
        {"ru", "🗑 Удалить упражнение"}, {"en", "🗑 Delete exercise"}, {"pl", "🗑 Usunąć ćwiczenie"}
    }},
    {"kg", {
        {"ru", "кг"}, {"en", "kg"}, {"pl", "kg"}
    }},

    {"last_results", {
        {"ru", "📝 Последние записи:"},
        {"en", "📝 Latest results:"},
        {"pl", "📝 Ostatnie wyniki:"},
    }},

    {"no_information", {
        {"ru", "Пока ничего нет 😕"},
        {"en", "Nothing yet 😕"},
        {"pl", "Na razie nic nie ma 😕"},
}},

// other things

{"back", {
    {"ru", "⬅ Назад"}, {"en", "⬅ Back"}, {"pl", "⬅ Wstecz"}
}},

{"select", {
        {"ru", "Выберите: "},
        {"en", "Select: "},
        {"pl", "Wybierz: "},
}},

{"your_profile", {
    {"ru", "📊 <b>Ваш профиль</b>"},
    {"en", "📊 <b>Your profile</b>"},
    {"pl", "📊 <b>Twój profil</b>"},
}},

{"time_spent", {
    {"ru", "⏳ Проведено времени: "},
    {"en", "⏳ Time spent: "},
    {"pl", "⏳ Spędzony czas: "},
}},

{"days", {{"ru", "д "}, {"en", "d "}, {"pl", "d "}}},
{"minutes", {{"ru", "м "}, {"en", "m "}, {"pl", "m "}}},
{"seconds", {{"ru", "с "}, {"en", "s "}, {"pl", "s "}}},

{"messages", {
    {"ru", "💬 Введено сообщений: "},
    {"en", "💬 Messages entered: "},
    {"pl", "💬 Wprowadzono wiadomości: "},
} },
    {"buttons", {
        {"ru", "🔘 Нажато кнопок: "},
        {"en", "🔘 Buttons pressed: "},
        {"pl", "🔘 Naciśnięto przycisk: "},
} },
    {"records", {
        {"ru", "🏆 Рекорды"},
        {"en", "🏆 Records"},
        {"pl", "🏆 Rekordy"},
} },
    {"records_by_category", {
    {"ru", "🏋️ <b>Рекорды: "},
    {"en", "🏋️ <b>Records: "},
    {"pl", "🏋️ <b>Rekordy: "},
} },

    {"max_weight", {
        {"ru", "🏋️ <b>Макс. вес:</b> "},
        {"en", "🏋️ <b>Max weight:</b> "},
        {"pl", "🏋️ <b>Maks. waga:</b>"},
} },

    {"reps", {
        {"ru", " раз, "},
        {"en", " reps, "},
        {"pl", " razy, "},
} },

    {"max_reps", {
        {"ru", "🔁 <b>Макс. повторений:</b> "},
        {"en", "🔁 <b>Max reps:</b> "},
        {"pl", "🔁 <b>Maks. powtórzeń:</b> "},
} },

    {"select_category", {
        {"ru", "🏆 <b>Выберите категорию</b>"},
        {"en", "🏆 <b>Select a category</b>"},
        {"pl", "🏆 <b>Wybierz kategorię</b>"},
} },

    {"no_information_category", {
        {"ru", "🙁 Нет данных по этой категории."},
        {"en", "🙁 No data available for this category."},
        {"pl", "🙁 Brak danych w tej kategorii."},
} },

    {"add_info", {
        {"ru", "➕ Добавить информацию"},
        {"en", "➕ Add information"},
        {"pl", "➕ Dodaj informacje"},
} },

    {"delete_info", {
        {"ru", "🗑 Удалить информацию"},
        {"en", "🗑 Delete information"},
        {"pl", "🗑 Usuń informacje"},
} },

    {"view_info", {
        {"ru", "🔍 Просмотреть"},
        {"en", "🔍 View"},
        {"pl", "🔍 Popatrzyć"},
} },

    {"delete_btn", {
        {"ru", "✅ Кнопка удалена!"},
        {"en", "✅ Button deleted!"},
        {"pl", "✅ Przycisk usunięty!"},
} },

    {"enter_weight", {
        {"ru", "Введите вес и количество повторений:"},
        {"en", "Enter the weight and number of reps:"},
        {"pl", "Wprowadź wagę i liczbę powtórzeń:"},
} },

    {"error_delete_btn", {
        {"ru", "❌ Ошибка при удалении кнопки!"},
        {"en", "❌ Error when removing the button!"},
        {"pl", "❌ Błąd podczas usuwania przycisku!"},
} },

    {"select_delete_btn", {
        {"ru", "Выберите кнопку для удаления:"},
        {"en", "Select the button to delete:"},
        {"pl", "Wybierz przycisk, aby usunąć:"},
} },

    {"enter_btn", {
        {"ru", "Введите название кнопки: "},
        {"en", "Enter the button name: "},
        {"pl", "Wpisz nazwę przycisku: "},
} },

    {"info_added", {
        {"ru", "✅ Данные добавлены!"},
        {"en", "✅ Data added!"},
        {"pl", "✅ Dane dodane!"},
} },

    {"btn_added", {
        {"ru", "✅ Кнопка добавлена!"},
        {"en", "✅ Button added!"},
        {"pl", "✅ Przycisk dodany!"},
} },

    {"start_error", {
        {"ru", "⚠️ Ошибка: ваш профиль не найден в базе. Напиши /start, чтобы зарегистрироваться."},
        {"en", "⚠️ Error: your profile was not found in the database. Type /start to register."},
        {"pl", "⚠️ Błąd: nie znaleziono Twojego profilu w bazie danych. Napisz /start, aby się zarejestrować."},
} },

    {"btn_name_error", {
        {"ru", "❌ Ошибка! Введите название кнопки"},
        {"en", "❌ Error! Enter the button name"},
        {"pl", "❌ Błąd! Wpisz nazwę przycisku"},
} },

    {"btn_name_repeat_error", {
        {"ru", "❌ Названия не могут повторяться!"},
        {"en", "❌ Names can't be the same!"},
        {"pl", "❌ Nazwy nie mogą się powtarzać!"},
} },

    {"no_data_error", {
        {"ru", "❌ Ошибка! Введите данные"},
        {"en", "❌ Error! Enter the data"},
        {"pl", "❌ Błąd! Wprowadź dane"},
} },

    {"no_training_data_error", {
        {"ru", "❌ Ошибка! Введите вес и количество повторений через пробел, например: 50 10"},
        {"en", "❌ Error! Enter the weight and number of reps separated by a space, for example: 50 10"},
        {"pl", "❌ Błąd! Wpisz wagę i liczbę powtórzeń, oddzielając je spacją, na przykład: 50 10"},
} },

    {"error", {
        {"ru", "❌ Ошибка!"},
        {"en", "❌ Error!"},
        {"pl", "❌ Błąd!"},
} },

    {"btn_added_error", {
        {"ru", "❌ Ошибка при добавлении кнопки: "},
        {"en", "❌ Error adding button: "},
        {"pl", "❌ Błąd podczas dodawania przycisku: "},
} },


};
std::string getTranslation(const std::string& key, const std::string& language) {
    auto itKey = translations.find(key);
    if (itKey != translations.end()) {
        auto itLang = itKey->second.find(language);
        if (itLang != itKey->second.end()) {
            return itLang->second;
        }
        itLang = itKey->second.find("en");
        if (itLang != itKey->second.end()) {
            return itLang->second;
        }
    }
    return key;
}
TgBot::InlineKeyboardButton::Ptr makeButton(const std::string& key, const std::string& callbackData, const std::string& lang) {
    auto btn = std::make_shared<TgBot::InlineKeyboardButton>();
    btn->text = getTranslation(key, lang);
    btn->callbackData = callbackData;
    return btn;
}