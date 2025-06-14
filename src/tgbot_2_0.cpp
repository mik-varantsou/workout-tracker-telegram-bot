#include "pch.h"
#include "config.h"
#include "database.h"
#include "keyboard_helpers.h"
#include "history.h"
#include "logic.h"



// creating main menu keyboard
TgBot::InlineKeyboardMarkup::Ptr getMainMenu(int64_t userId, SQLite::Database& db) {
    addToHistory(userId, "menu_back");
    std::string lang = getLanguage(userId, db);

    auto keyboard = TgBot::InlineKeyboardMarkup::Ptr(new TgBot::InlineKeyboardMarkup);

    auto menu_back = TgBot::InlineKeyboardButton::Ptr(new TgBot::InlineKeyboardButton);
    menu_back->text = "menu_back";
    menu_back->callbackData = "menu_back";


    keyboard->inlineKeyboard.push_back({
        makeButton("menu_profile", "profile", lang)
        });
    keyboard->inlineKeyboard.push_back({
        makeButton("menu_training", "training", lang)
        });
    keyboard->inlineKeyboard.push_back({
        makeButton("lang_en", "en", lang),
        makeButton("lang_pl", "pl", lang),
        makeButton("lang_ru", "ru", lang)
        });

    return keyboard;
}

// creating and translating all /commands  
void setCommandsForUser(const std::string& language, TgBot::Bot& bot) {
    std::vector<TgBot::BotCommand::Ptr> commands;
    auto createCommand = [](const std::string& cmd, const std::string& desc) {
        auto command = std::make_shared<TgBot::BotCommand>();
        command->command = cmd;
        command->description = desc;
        return command;
        };

    commands.push_back(createCommand("menu", getTranslation("menu", language)));
    commands.push_back(createCommand("profile", getTranslation("menu_profile", language)));
    commands.push_back(createCommand("start", getTranslation("start", language)));
    commands.push_back(createCommand("chest", getTranslation("chest", language)));
    commands.push_back(createCommand("back", getTranslation("spine", language)));
    commands.push_back(createCommand("arms", getTranslation("arms", language)));
    commands.push_back(createCommand("abs", getTranslation("abs", language)));
    commands.push_back(createCommand("legs", getTranslation("legs", language)));

    bot.getApi().setMyCommands(commands);
}

//function for getting time irl
std::string dateInfo() {
    auto now = std::chrono::system_clock::now();
    auto local_tm = std::chrono::current_zone()->to_local(now);
    return std::format("{:%d.%m.%Y %H:%M}", local_tm);
}


void removeDeletedChats(TgBot::Bot& bot, SQLite::Database& db) {
    try {
        SQLite::Statement query(db, "SELECT user_id FROM users");
        std::vector<int64_t> toDelete;  // Список чатов для удаления

        std::cout << "🔎 Проверяем чаты в базе..." << std::endl;

        while (query.executeStep()) {
            int64_t chatId = query.getColumn(0).getInt64();
            std::cout << "➡ Проверяем чат: " << chatId << std::endl;

            try {
                bot.getApi().getChat(chatId);  // Проверяем доступность
            }
            catch (const TgBot::TgException& e) {
                std::string errorMsg = e.what();
                if (errorMsg.find("Forbidden") != std::string::npos ||
                    errorMsg.find("chat not found") != std::string::npos) {

                    std::cerr << "❌ Чат " << chatId << " удалён или заблокирован!" << std::endl;
                    toDelete.push_back(chatId);
                }
            }
        }

        // Удаляем все найденные удалённые чаты
        for (int64_t chatId : toDelete) {
            std::cout << "🗑 Удаляем из базы: " << chatId << std::endl;
            SQLite::Statement deleteQuery(db, "DELETE FROM users WHERE user_id = ?");
            deleteQuery.bind(1, chatId);
            deleteQuery.exec();
        }

        std::cout << "✅ Очистка завершена! Удалено чатов: " << toDelete.size() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "⚠ Ошибка при очистке удалённых чатов: " << e.what() << std::endl;
    }
}

int main() {
    std::string token = readEnvToken(".env", "BOT_TOKEN");
    if (token.empty()) {
        std::cerr << "Bot token not found in .env file!" << std::endl;
        std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
        return 1;
    }

    TgBot::Bot bot(token); // теперь токен из .env

    db_path = (std::filesystem::current_path() / DATABASE_PATH).string();
    db = SQLite::Database(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    createDatabase(db_path); // если создаёт структуру в БД
    removeDeletedChats(bot, db);

    struct CallbackData {
        std::string type;              // type of action: btn, add, delete_start etc
        std::string exercise;          // for example, chest
        int buttonIndex = -1;          
        int subSectionIndex = -1;     
        std::string raw;

        CallbackData(const std::string& data) : raw(data) {
            parse();
        }

    private:
        void parse() {
            if (raw.rfind("btn_", 0) == 0) {
                type = "btn";
                auto parts = split(raw.substr(4), '_');
                if (parts.size() >= 2) {
                    exercise = parts[0];
                    buttonIndex = std::stoi(parts[1]);
                    if (parts.size() == 3) {
                        subSectionIndex = std::stoi(parts[2]);
                    }
                }
            }
            else if (raw.rfind("custom_", 0) == 0) {
                auto parts = split(raw, ':');
                if (parts.size() == 2) {
                    type = parts[0]; 
                    exercise = parts[1];
                }
                else {
                    type = raw;
                }
            }
            else if (raw.rfind("add:", 0) == 0) {
                type = "add";
                exercise = raw.substr(4);
            }
            else if (raw.rfind("delete_start:", 0) == 0) {
                type = "delete_start";
                exercise = raw.substr(13);
            }
            else if (raw.rfind("delete_end:", 0) == 0) {
                type = "delete_end";
                exercise = raw.substr(11);
            }
            else if (raw.rfind("back_btn:", 0) == 0) {
                type = "back_btn";
                exercise = raw.substr(5);
            }
            else {
                type = raw;
            }
        }

        // helper for spliting the line
        std::vector<std::string> split(const std::string& s, char delimiter) {
            std::vector<std::string> tokens;
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delimiter)) {
                tokens.push_back(item);
            }
            return tokens;
        }
    };


    // button start and adding the user to the database
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, getTranslation("start", getLanguage(message->chat->id, db)) , nullptr, 0, getMainMenu(message->chat->id, db));
        setCommandsForUser(getLanguage(message->chat->id, db), bot);

        // adding user
        try {
            onUserMessage(message->chat->id);
            SQLite::Statement checkQuery(db, "SELECT 1 FROM users WHERE user_id = ?");
            checkQuery.bind(1, message->chat->id);

            if (!checkQuery.executeStep()) {
                SQLite::Statement query(db, "INSERT INTO users(user_id) VALUES (?)");
                query.bind(1, message->chat->id);
                query.exec();

            }

        }
        catch (std::exception& e) {
            std::cerr << "Database error: " << e.what() << std::endl;
            bot.getApi().sendMessage(message->chat->id, "Error. Please, try later.");
        }
        });
    bot.getEvents().onCommand("profile", [&bot](TgBot::Message::Ptr message) {
        handleProfile(message->chat->id, -1, bot);
        });
    bot.getEvents().onCommand("menu", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, getTranslation("menu", getLanguage(message->chat->id, db)) + "\n", nullptr, 0, getMainMenu(message->chat->id, db));
        });
    bot.getEvents().onCommand("chest", [&bot](TgBot::Message::Ptr message) {
        handleExercise(message->chat->id, -1, bot, "chest");
        });
    bot.getEvents().onCommand("back", [&bot](TgBot::Message::Ptr message) {
        handleExercise(message->chat->id, -1, bot, "spine");
        });
    bot.getEvents().onCommand("arms", [&bot](TgBot::Message::Ptr message) {
        handleExercise(message->chat->id, -1, bot, "arms");
        });
    bot.getEvents().onCommand("abs", [&bot](TgBot::Message::Ptr message) {
        handleExercise(message->chat->id, -1, bot, "abs");
        });
    bot.getEvents().onCommand("legs", [&bot](TgBot::Message::Ptr message) {
        handleExercise(message->chat->id, -1, bot, "legs");
        });


    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        int64_t userId = message->chat->id;
        std::string language = getLanguage(userId, db);
        SQLite::Statement query(db, "SELECT state, exercise FROM users WHERE user_id = ?");
        query.bind(1, userId);
        if (!query.executeStep()) {
            bot.getApi().sendMessage(userId, getTranslation("error_start", language));
            return;
        }
        onUserMessage(userId);
        std::string state = query.getColumn(0).getText();
        std::string exercise = query.getColumn(1).getText();
        int32_t messageId = message->messageId;
        TgBot::InlineKeyboardMarkup::Ptr back(new TgBot::InlineKeyboardMarkup);
        std::cout << userId << std::endl;


        try {
                std::string state = query.getColumn(0).getText();
                std::string exercise = query.getColumn(1).getText();


                if (state == "waiting_button") {
                    std::cout << state << std::endl;
                    std::string buttonName = message->text;
                    if (buttonName.empty()) {
                        bot.getApi().sendMessage(userId, getTranslation("btn_name_error", language));
                        return;
                    }



                    std::string prefix = "btn_" + exercise + "_";
                    int prefixLength = prefix.size();

                    SQLite::Statement maxQuery(db,
                        "SELECT MAX(CAST(SUBSTR(callback_data, ? ) AS INTEGER)) "
                        "FROM custom_buttons WHERE user_id = ? AND callback_data LIKE ?");
                    maxQuery.bind(1, static_cast<int>(prefixLength + 1));
                    maxQuery.bind(2, userId);
                    maxQuery.bind(3, prefix + "%");

                    int buttonIndex = 0;
                    if (maxQuery.executeStep()) {
                        if (!maxQuery.getColumn(0).isNull()) {
                            buttonIndex = maxQuery.getColumn(0).getInt();
                        }
                    }

                    std::string callbackData = prefix + std::to_string(buttonIndex + 1);



                    addToHistory(userId, "menu_back");
                    if (saveUserButton(userId, buttonName, callbackData, db)) {
                        bot.getApi().sendMessage(userId, getTranslation("btn_added", language), nullptr, 0, createBackKeyboard(userId));
                        setState(userId, db, "resting");
                    }
                    else {
                        bot.getApi().sendMessage(userId, getTranslation("btn_name_repeat_error", language), nullptr, 0, createBackKeyboard(userId));
                    }

                }

                if (state == "waiting_weight_reps") {
                    std::string button_callback = getButtonPressed(userId, db);
                    std::string button_name = getCustomButtonName(userId, db, button_callback);

                    SQLite::Statement maxQuery(db,
                        "SELECT MAX(CAST(SUBSTR(callback_data, " +
                        std::to_string(button_callback.length() + 1) +
                        ") AS INTEGER)) FROM user_trainings WHERE user_id = ? AND callback_data LIKE ?");

                    maxQuery.bind(1, userId);
                    maxQuery.bind(2, button_callback + "%");

                    int nextNumber = 0;
                    if (maxQuery.executeStep() && !maxQuery.isColumnNull(0)) {
                        nextNumber = maxQuery.getColumn(0).getInt() + 1;
                    }


                    std::string callbackData = button_callback + "_" + std::to_string(nextNumber);


                    std::cout << state << std::endl;
                    std::string weightReps = message->text;
                    if (weightReps.empty()) {
                        bot.getApi().sendMessage(userId, getTranslation("no_data_error", language));
                        return;
                    }
                    std::istringstream iss(weightReps);
                    std::string weight, reps, rest;
                    if (!(iss >> weight >> reps) || (iss >> rest)) {
                        bot.getApi().sendMessage(userId, getTranslation("no_training_data_error", language));
                        return;
                    }

                    if (saveUserResults(userId, db, weight, reps, dateInfo(), button_name, callbackData)) {
                        bot.getApi().sendMessage(userId, getTranslation("info_added", language), nullptr, 0, createBackKeyboard(userId));
                        setState(userId, db, "resting");
                    }
                    else {
                        bot.getApi().sendMessage(userId, getTranslation("error", language), nullptr, 0, createBackKeyboard(userId));
                    }



                }
            }


        catch (const TgBot::TgException& e) {
                std::string errorMsg = e.what();
                if (errorMsg.find("bot was blocked by the user") != std::string::npos) {
                    std::cout << "⚠️ User " << userId << " blocked the bot." << std::endl;
                    return;
                }
                bot.getApi().sendMessage(userId, getTranslation("btn_added_error", language) + errorMsg);
            }
        });    
    bot.getEvents().onCallbackQuery([&bot](TgBot::CallbackQuery::Ptr query) {
        int64_t userId = query->message->chat->id;
        int32_t messageId = query->message->messageId;
        std::string callbackData = query->data;
        std::cout << callbackData << std::endl;
        onUserClick(userId);
        std::string language = getLanguage(userId, db);

        CallbackData cd(callbackData);


        std::unordered_map<std::string, std::function<void(const CallbackData&)>> handlers = {

            // switch languages 
            {"pl",[&](const CallbackData&) {
                setLanguage(userId, db, "pl");
                setCommandsForUser("pl", bot);
                bot.getApi().editMessageText(getTranslation("menu", "pl"), userId, messageId, "", "", nullptr, getMainMenu(userId, db));

            }},
            {"ru",[&](const CallbackData&) {
                setLanguage(userId, db, "ru");
                setCommandsForUser("ru", bot);
                bot.getApi().editMessageText(getTranslation("menu", "ru"), userId, messageId, "", "", nullptr, getMainMenu(userId, db));
            }},
            {"en",[&](const CallbackData&) {
                setLanguage(userId, db, "en");
                setCommandsForUser("en", bot);
                bot.getApi().editMessageText(getTranslation("menu", "en"), userId, messageId, "", "", nullptr, getMainMenu(userId, db));
            }},

            {"menu", [&](const CallbackData&) {
                bot.getApi().editMessageText(getTranslation("menu", language) , userId, messageId, "", "", nullptr, getMainMenu(userId, db));
            }},

            {"profile",[&](const CallbackData&) {
                handleProfile(userId, messageId, bot);
            }},

            {"records",[&](const CallbackData&) {
                handleRecords(userId, messageId, bot);
            }},

            {"records_cat_chest",[&](const CallbackData&) {
                handleRecordsByCategory(userId, messageId, bot, "chest");
            }},

            {"records_cat_spine",[&](const CallbackData&) {
                handleRecordsByCategory(userId, messageId, bot, "spine");
            }},

            {"records_cat_arms",[&](const CallbackData&) {
                handleRecordsByCategory(userId, messageId, bot, "arms");
            }},

            {"records_cat_abs",[&](const CallbackData&) {
                handleRecordsByCategory(userId, messageId, bot, "abs");
            }},

            {"records_cat_legs",[&](const CallbackData&) {
                handleRecordsByCategory(userId, messageId, bot, "legs");
            }},

            {"training", [&](const CallbackData&) {
                processState(userId, messageId, "training", {
                    {getTranslation("chest", language), "chest"},
                    {getTranslation("spine", language), "spine"},
                    {getTranslation("legs", language), "legs"},
                    {getTranslation("arms", language), "arms"},
                    {getTranslation("abs", language), "abs"},
                    });
            }},

            {"chest", [&](const CallbackData&) {
                handleExercise(userId, messageId, bot, "chest");
            }},

            {"spine", [&](const CallbackData&) {
                handleExercise(userId, messageId, bot, "spine");
            }},

            {"arms", [&](const CallbackData&) {
                handleExercise(userId, messageId, bot, "arms");
            }},

            {"abs", [&](const CallbackData&) {
                handleExercise(userId, messageId, bot, "abs");
            }},

            {"legs", [&](const CallbackData&) {
                handleExercise(userId, messageId, bot, "legs");
            }},

            {"add", [&](const CallbackData& data) {
                setState(userId, db, "waiting_button");
                setExercise(userId, db, data.exercise);
                bot.getApi().editMessageText(getTranslation("enter_btn", language), userId, messageId);
            }},

            {"delete_start", [&](const CallbackData& data) {
                setState(userId, db, "deleting");
                setExercise(userId, db, data.exercise);
                TgBot::InlineKeyboardMarkup::Ptr emptyF(new TgBot::InlineKeyboardMarkup);
                TgBot::InlineKeyboardMarkup::Ptr keyboard = mergeKeyboards(userId, "btn_" + data.exercise, emptyF);
                addToHistory(userId, "delete_start");
                keyboard->inlineKeyboard.push_back({ createBackButton(userId, getPreviousState(userId)) });
                bot.getApi().editMessageText(getTranslation("select_delete_btn", language), userId, messageId, "", "", nullptr, keyboard);
            }},

            {"delete_end", [&](const CallbackData& data) {
                if (deleteUserButton(userId, data.exercise, db)) {
                    bot.getApi().editMessageText(getTranslation("delete_btn", language), userId, messageId, "", "", nullptr, createBackKeyboard(userId));
                }
                else {
                    bot.getApi().editMessageText(getTranslation("error_delete_btn", language), userId, messageId, "", "", nullptr, createBackKeyboard(userId));
                }
            }},

            {"btn", [&](const CallbackData& data) {
                setButtonPressed(userId, db, data.raw);  // сохраняем callback
                if (data.exercise == "chest") {
                    processState(userId, messageId, data.raw, {
                    {getTranslation("add_info", language), "custom_add:" + data.raw},
                    {getTranslation("delete_info", language), "custom_delete_start:" + data.raw},
                    {getTranslation("view_info", language), "custom_view:" + data.raw}
                        });
                }
                if (data.exercise == "spine") {
                    processState(userId, messageId, data.raw, {
                    {getTranslation("add_info", language), "custom_add:" + data.raw},
                    {getTranslation("delete_info", language), "custom_delete_start:" + data.raw},
                    {getTranslation("view_info", language), "custom_view:" + data.raw}
                        });
                }
                if (data.exercise == "arms") {
                    processState(userId, messageId, data.raw, {
                    {getTranslation("add_info", language), "custom_add:" + data.raw},
                    {getTranslation("delete_info", language), "custom_delete_start:" + data.raw},
                    {getTranslation("view_info", language), "custom_view:" + data.raw}
                        });
                }
                if (data.exercise == "abs") {
                    processState(userId, messageId, data.raw, {
                    {getTranslation("add_info", language), "custom_add:" + data.raw},
                    {getTranslation("delete_info", language), "custom_delete_start:" + data.raw},
                    {getTranslation("view_info", language), "custom_view:" + data.raw}
                        });
                }
                if (data.exercise == "legs") {
                    processState(userId, messageId, data.raw, {
                    {getTranslation("add_info", language), "custom_add:" + data.raw},
                    {getTranslation("delete_info", language), "custom_delete_start:" + data.raw},
                    {getTranslation("view_info", language), "custom_view:" + data.raw}
                        });
                }

            }},

            {"custom_add", [&](const CallbackData& data) {
                setState(userId, db, "waiting_weight_reps");
                addToHistory(userId, "custom_add");
                bot.getApi().editMessageText(getTranslation("enter_weight", language), userId, messageId);
            }},

            {"custom_view", [&](const CallbackData& data) {
                std::string buttonName = getCustomButtonName(userId, db, data.exercise);
                viewUserResults(userId, db, buttonName);
            }},

            {"custom_delete_start", [&](const CallbackData& data) {
                std::string buttonName = getCustomButtonName(userId, db, data.exercise);
                processState(userId, messageId, "custom_delete_start", {}, true, data.exercise + "_%", true, buttonName);
            }},

            {"custom_delete_end", [&](const CallbackData& data) {
                deleteUserResults(userId, db, data.exercise);
                bot.getApi().editMessageText(getTranslation("delete_btn", language), userId, messageId, "", "", nullptr, createBackKeyboard(userId));
            }},

            {"back_btn", [&](const CallbackData& data) {
                std::string prevState = goBack(userId);

                if (handlers.find(prevState) != handlers.end()) {
                    handlers[prevState](data);
                }
    // Если это кнопка вида btn_chest_3
                else if (prevState.rfind("btn_", 0) == 0) {
                    CallbackData btnData(prevState);
                    handlers["btn"](btnData);
                }
    // Иначе — fallback в главное меню
                else {
                    bot.getApi().editMessageText(getTranslation("menu", language), userId, messageId, "", "", nullptr, getMainMenu(userId, db));
                }
            }
        }

        };

        try {
            if (handlers.find(cd.type) != handlers.end()) {
                handlers[cd.type](cd);
            }
            else {
                bot.getApi().sendMessage(userId, "⚠️ Unknown request: " + cd.type);
            }
        }
        catch (const std::exception& e) {
           // bot.getApi().sendMessage(userId, "⚠️ Ошибка: " + std::string(e.what()));
        }
        });
        
        
        //long polling updating telegram requests
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            try {
                printf("Long poll started\n");
                longPoll.start();
            }
            catch (const TgBot::TgException& e) {
                std::string errorMsg = e.what();
                std::cerr << "Ошибка TgBot: " << errorMsg << std::endl;

                if (errorMsg.find("bot was blocked by the user") != std::string::npos) {
                    std::cout << "⚠️ User blocked the bot." << std::endl;
                }

                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
            catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
            catch (...) {
                std::cerr << "Unknown error!" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }
        return 0;  // long polling is infinite, because of that code will never reach that point
}

