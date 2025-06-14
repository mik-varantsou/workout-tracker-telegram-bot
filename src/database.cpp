#include "database.h"




// creating a database if it does not already exist
void createDatabase(const std::string& db_path) {
    try {
        SQLite::Database db(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        db.exec("PRAGMA foreign_keys = ON;");

        // table of users and their statuses
        db.exec("CREATE TABLE IF NOT EXISTS users ("
            "user_id INTEGER PRIMARY KEY,"
            "state TEXT DEFAULT '',"
            "exercise TEXT DEFAULT '',"
            "button_pressed TEXT DEFAULT '',"
            "language TEXT DEFAULT ''"
            ")");

        db.exec("CREATE TABLE IF NOT EXISTS user_stats ("
            "user_id INTEGER PRIMARY KEY,"
            "created_at INTEGER,"
            "last_active INTEGER,"
            "message_count INTEGER DEFAULT 0,"
            "button_count INTEGER DEFAULT 0"
            ")");

        // table of custom buttons
        db.exec("CREATE TABLE IF NOT EXISTS custom_buttons ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "user_id INTEGER,"
            "button_name TEXT,"
            "callback_data TEXT,"
            "FOREIGN KEY(user_id) REFERENCES users(user_id)"
            ")");

        // table of exercise records
        db.exec("CREATE TABLE IF NOT EXISTS user_trainings ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "user_id INTEGER,"
            "exercise_name TEXT,"
            "weight INTEGER,"
            "reps INTEGER,"
            "training_date TEXT,"  // лучше заменить на INTEGER, если используешь timestamp
            "callback_data TEXT,"
            "FOREIGN KEY(user_id) REFERENCES users(user_id),"
            "FOREIGN KEY(exercise_name) REFERENCES custom_buttons(button_name) ON DELETE CASCADE"
            ")");

        db.exec("CREATE TRIGGER IF NOT EXISTS delete_user_trainings_after_button_deleted "
            "AFTER DELETE ON custom_buttons "
            "FOR EACH ROW "
            "BEGIN "
            "    DELETE FROM user_trainings "
            "    WHERE exercise_name = OLD.button_name; "
            "END;");

        std::cout << "Database opened successfully!\n";
    }
    catch (std::exception& e) {
        std::cerr << "SQLite error: " << e.what() << std::endl;
        std::cerr << "Path to the database: " << db_path << std::endl;
    }
}

bool saveUserButton(int64_t userId, const std::string& buttonName, const std::string& callbackData, SQLite::Database &db) {
    try {
        SQLite::Statement stmt(db, "INSERT INTO custom_buttons (user_id, button_name, callback_data) VALUES (?, ?, ?)");
        stmt.bind(1, userId);
        stmt.bind(2, buttonName);
        stmt.bind(3, callbackData);
        stmt.exec();
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при выполнении запроса: " << e.what() << std::endl;
        return false;
    }
}
bool deleteUserButton(int64_t userId, const std::string& callbackData,  SQLite::Database& db) {
    try {
        SQLite::Statement stmt(db, "DELETE FROM custom_buttons WHERE user_id = ? AND callback_data = ?");
        stmt.bind(1, userId);
        stmt.bind(2, callbackData); 
        stmt.exec();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при выполнении запроса: " << e.what() << std::endl;
        return false;
    }
}

bool saveUserResults(int64_t userId, SQLite::Database& db, std::string weight, std::string reps, std::string training_date, std::string buttonName, std::string callback_data){
    try {
        SQLite::Statement stmt(db, "INSERT INTO user_trainings (user_id, exercise_name, weight, reps, training_date, callback_data) VALUES (?, ?, ?, ?, ?, ?)");
        stmt.bind(1, userId);
        stmt.bind(2, buttonName);
        stmt.bind(3, weight);
        stmt.bind(4, reps);
        stmt.bind(5, training_date);
        stmt.bind(6, callback_data); ///////////////////////////////////////////////////////////////
        stmt.exec();
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при выполнении запроса: " << e.what() << std::endl;
        return false;
    }
}
bool viewUserResults(int64_t userId, SQLite::Database& db, const std::string& button_name)
{
    try {
        SQLite::Statement stmt(db,
            "SELECT weight, reps, training_date FROM user_trainings WHERE user_id = ? AND exercise_name = ? ORDER BY training_date DESC LIMIT 10");
        stmt.bind(1, userId);
        stmt.bind(2, button_name);
        std::string language = getLanguage(userId, db);

        std::string messageText = getTranslation("last_results", language)+ "\n";
        bool hasResults = false;

        while (stmt.executeStep()) {
            hasResults = true;
            std::string weight = stmt.getColumn(0).getString();
            std::string reps = stmt.getColumn(1).getString();
            std::string time = stmt.getColumn(2).getString();

            messageText += "• " + button_name + " — " +
                weight + getTranslation("kg", language) + " × " +
                reps + " (" + time + ")\n";
        }

        if (!hasResults) {
            messageText += getTranslation("no_information", language);
        }

        bot.getApi().sendMessage(userId, messageText);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при выполнении запроса: " << e.what() << std::endl;
        return false;
    }
}
bool deleteUserResults(int64_t userId, SQLite::Database& db, const std::string& callbackData)
{
    try {
        SQLite::Statement stmt(db, "DELETE FROM user_trainings WHERE user_id = ? AND callback_data = ?");
        stmt.bind(1, userId);
        stmt.bind(2, callbackData);
        stmt.exec();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при выполнении запроса: " << e.what() << std::endl;
        return false;
    }
}


//checking if callback of the button is custom or not
bool isCustomUserSection(const std::string& callback, SQLite::Database& db, int64_t userId) {
    try {
        SQLite::Statement stmt(db,
            "SELECT 1 FROM custom_buttons WHERE user_id = ? AND callback_data = ?");
        stmt.bind(1, userId);
        stmt.bind(2, callback);
        return stmt.executeStep();
    }
    catch (const std::exception& e) {
        std::cerr << "Error with custom section: " << e.what() << std::endl;
        return false;
    }
}




std::string getSingleStringValue(SQLite::Database& db, const std::string& query, int64_t userId, const std::string& secondParam) {
    try {
        SQLite::Statement stmt(db, query);
        stmt.bind(1, userId);
        if (!secondParam.empty()) {
            stmt.bind(2, secondParam);
        }
        if (stmt.executeStep() && !stmt.isColumnNull(0)) {
            return stmt.getColumn(0).getString();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "SQLite error: " << e.what() << std::endl;
    }
    return "";
}
std::string getButtonPressed(int64_t userId, SQLite::Database& db) {
    return getSingleStringValue(db, "SELECT button_pressed FROM users WHERE user_id = ?", userId);
}
std::string getCustomButtonName(int64_t userId, SQLite::Database& db, const std::string& callback_data) {
    return getSingleStringValue(db, "SELECT button_name FROM custom_buttons WHERE user_id = ? AND callback_data = ?", userId, callback_data);
}
std::string getState(int64_t userId, SQLite::Database& db) {
    return getSingleStringValue(db, "SELECT state FROM users WHERE user_id = ?", userId);
}
std::string getLanguage(int64_t userId, SQLite::Database& db) {
    return getSingleStringValue(db, "SELECT language FROM users WHERE user_id = ?", userId);
}


void setUserField(int64_t userId, SQLite::Database& db, const std::string& fieldName, const std::string& value) {
    std::string query = "UPDATE users SET " + fieldName + " = ? WHERE user_id = ?";
    SQLite::Statement stmt(db, query);
    stmt.bind(1, value);
    stmt.bind(2, userId);
    stmt.exec();
}
void setState(int64_t userId, SQLite::Database& db, const std::string& state) {
    setUserField(userId, db, "state", state);
}
void setExercise(int64_t userId, SQLite::Database& db, const std::string& exercise) {
    setUserField(userId, db, "exercise", exercise);
}
void setButtonPressed(int64_t userId, SQLite::Database& db, const std::string& callback_data) {
    setUserField(userId, db, "button_pressed", callback_data);
}
void setLanguage(int64_t userId, SQLite::Database& db, const std::string& language) {
    setUserField(userId, db, "language", language);
}


