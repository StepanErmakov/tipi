#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

class ConfigManager {
private:
    std::map<std::string, std::string> settings;
    std::string currentFilename;
    bool modified;

    //вектор функций обратного вызова для уведомлений
    std::vector<std::function<void(const std::string&, const std::string&)>> observers;

    //валидация ключа
    bool validateKey(const std::string& key) const {
        if (key.empty()) return false;
        // Ключ не должен содержать пробелы и символ '='
        if (key.find_first_of(" \t\n\r=") != std::string::npos) return false;
        return true;
    }

protected:
    //валидация значения 
    virtual bool validateValue(const std::string& key, const std::string& value) const {
        //базовая валидация тк значение может быть пустым
        return true;
    }

    //уведомление наблюдателей
    void notifyObservers(const std::string& key, const std::string& newValue) {
        for (const auto& observer : observers) {
            observer(key, newValue);
        }
    }

public:
    ConfigManager() : modified(false) {}

    virtual ~ConfigManager() = default;

    //загрузка из файла
    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        std::map<std::string, std::string> newSettings;
        std::string line;
        int lineNum = 0;

        while (std::getline(file, line)) {
            lineNum++;

            //пропускаем пустые строки и комментарии
            if (line.empty() || line[0] == '#') continue;

            size_t equalsPos = line.find('=');
            if (equalsPos == std::string::npos) {
                throw std::runtime_error("Invalid format (missing '=') at line " + std::to_string(lineNum));
            }

            std::string key = line.substr(0, equalsPos);
            std::string value = line.substr(equalsPos + 1);

            //удаляем пробелы
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (key.empty()) {
                throw std::runtime_error("Empty key at line " + std::to_string(lineNum));
            }

            if (!validateKey(key)) {
                throw std::runtime_error("Invalid key format at line " + std::to_string(lineNum));
            }

            if (!validateValue(key, value)) {
                throw std::runtime_error("Invalid value for key '" + key + "' at line " + std::to_string(lineNum));
            }

            newSettings[key] = value;
        }

        settings = std::move(newSettings);
        currentFilename = filename;
        modified = false;
        file.close();
    }

    //получение настройки
    std::string getSetting(const std::string& key) const {
        auto it = settings.find(key);
        if (it != settings.end()) {
            return it->second;
        }
        throw std::runtime_error("Setting not found: " + key);
    }

    //установка настройки
    void setSetting(const std::string& key, const std::string& value) {
        if (!validateKey(key)) {
            throw std::invalid_argument("Invalid key format: " + key);
        }

        if (!validateValue(key, value)) {
            throw std::invalid_argument("Invalid value for key: " + key);
        }

        auto it = settings.find(key);
        bool isNewKey = (it == settings.end());
        std::string oldValue = isNewKey ? "" : it->second;

        settings[key] = value;
        modified = true;

        if (isNewKey || oldValue != value) {
            notifyObservers(key, value);
        }
    }

    void saveChanges() {
        if (!modified) {
            std::cout << "No changes to save" << std::endl;
            return;
        }

        if (currentFilename.empty()) {
            throw std::runtime_error("No file loaded to save changes");
        }

        //временный файл для безопасной записи
        std::string tempFilename = currentFilename + ".tmp";
        std::ofstream file(tempFilename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open temporary file for writing: " + tempFilename);
        }

        for (const auto& pair : settings) {
            file << pair.first << " = " << pair.second << std::endl;
        }

        file.close();

        //переименовываем временный файл в основной
        if (std::rename(tempFilename.c_str(), currentFilename.c_str()) != 0) {
            throw std::runtime_error("Failed to rename temporary file");
        }

        modified = false;
        std::cout << "Changes saved to " << currentFilename << std::endl;
    }

    void saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file for writing: " + filename);
        }

        for (const auto& pair : settings) {
            file << pair.first << " = " << pair.second << std::endl;
        }

        modified = false;
        currentFilename = filename;
        file.close();
        std::cout << "Settings saved to " << filename << std::endl;
    }

    //добавляем наблюдателя
    void addObserver(std::function<void(const std::string&, const std::string&)> observer) {
        if (observer) {
            observers.push_back(observer);
        }
    }

    //проверяем наличия настройки
    bool hasSetting(const std::string& key) const {
        return settings.find(key) != settings.end();
    }

    //удаляем настройки
    void removeSetting(const std::string& key) {
        auto it = settings.find(key);
        if (it != settings.end()) {
            settings.erase(it);
            modified = true;
            std::cout << "Setting '" << key << "' removed" << std::endl;
        }
    }

    //получение всех ключей
    std::vector<std::string> getAllKeys() const {
        std::vector<std::string> keys;
        keys.reserve(settings.size());
        for (const auto& pair : settings) {
            keys.push_back(pair.first);
        }
        return keys;
    }

    //проверяем изменения
    bool isModified() const {
        return modified;
    }

    //получение имени файла
    std::string getCurrentFilename() const {
        return currentFilename;
    }

    //очищаем все настройки
    void clear() {
        settings.clear();
        modified = true;
        notifyObservers("__clear__", "");
    }
};

//специальный класс для приложения
class AppConfigManager : public ConfigManager {
private:
    bool validateValue(const std::string& key, const std::string& value) const override {
        if (key == "port") {
            if (value.empty()) return false;
            try {
                int port = std::stoi(value);
                return port >= 1 && port <= 65535;
            }
            catch (...) {
                return false;
            }
        }

        if (key == "debug") {
            return value == "true" || value == "false";
        }

        if (key == "timeout") {
            if (value.empty()) return false;
            try {
                int timeout = std::stoi(value);
                return timeout > 0;
            }
            catch (...) {
                return false;
            }
        }

        if (key == "server") {
            return !value.empty();
        }

        return true; 
    }

public:
    //метод для пакетного обновления
    void updateSettings(const std::map<std::string, std::string>& newSettings) {
        for (const auto& pair : newSettings) {
            setSetting(pair.first, pair.second);
        }
    }
};

int main() {
    try {
        AppConfigManager config;

        //добавляем наблюдателей
        config.addObserver([](const std::string& key, const std::string& value) {
            std::cout << "[Observer] " << key << " = " << value << std::endl;
            });

        //устанавливаем настройки
        config.setSetting("server", "localhost");
        config.setSetting("port", "8080");
        config.setSetting("debug", "false");
        config.setSetting("timeout", "30");

        config.saveToFile("config.txt");

        config.setSetting("debug", "true");
        config.setSetting("port", "9090");

        config.saveChanges();

        AppConfigManager config2;
        config2.loadFromFile("config.txt");

        std::cout << "\nLoaded settings:" << std::endl;
        for (const auto& key : config2.getAllKeys()) {
            std::cout << "  " << key << " = " << config2.getSetting(key) << std::endl;
        }

        //проверка валидации
        try {
            config2.setSetting("port", "99999"); 
        }
        catch (const std::exception& e) {
            std::cout << "\nValidation OK: " << e.what() << std::endl;
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}