#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

//БАЗОВЫЙ КЛАСС
class Course {
protected:
    std::string name;
    int duration;
    std::map<std::string, int> grades;

public:
    Course(std::string n, int d) : name(n), duration(d) {}
    virtual ~Course() = default;

    virtual void start() = 0;
    virtual void finish() = 0;

    void addGrade(const std::string& student, int grade) {
        grades[student] = grade;
    }

    int getGrade(const std::string& student) {
        auto it = grades.find(student);
        if (it != grades.end()) return it->second;
        return -1; //студент не найден
    }

    double getAverage() {
        if (grades.empty()) return 0;
        double sum = 0;
        for (auto& g : grades) sum += g.second;
        return sum / grades.size();
    }

    std::string getName() { return name; }
};

// ЛЕКЦИЯ 
class Lecture : public Course {
    std::string topic;
public:
    Lecture(std::string n, int d, std::string t) : Course(n, d), topic(t) {}

    void start() override {
        std::cout << " Лекция " << name << " на тему " << topic << "\n";
    }

    void finish() override {
        std::cout << "Лекция " << name << " завершена\n";
    }
};

// ЛАБОРАТОРНАЯ 
class LabWork : public Course {
    std::vector<std::string> tools;
public:
    LabWork(std::string n, int d) : Course(n, d) {}

    void addTool(std::string t) {
        tools.push_back(t);
    }

    void start() override {
        std::cout << " Лабораторная " << name << "\nИнструменты: ";
        if (tools.empty()) {
            std::cout << "не требуются";
        }
        else {
            for (size_t i = 0; i < tools.size(); i++) {
                std::cout << tools[i];
                if (i < tools.size() - 1) std::cout << ", ";
            }
        }
        std::cout << "\n";
    }

    void finish() override {
        std::cout << " Лабораторная " << name << " завершена\n";
    }
};

// ЭКЗАМЕН 
class Exam : public Course {
    std::string date;
public:
    Exam(std::string n, int d, std::string dt) : Course(n, d), date(dt) {}

    void start() override {
        std::cout << " Экзамен " << name << " " << date << "\n";
    }

    void finish() override {
        std::cout << " Экзамен " << name << " завершен\n";
    }
};

//  РАСПИСАНИЕ 
class Schedule {
private:
    struct Item {
        std::shared_ptr<Course> course;
        std::string day;
        std::string time;
        std::string room;
    };
    std::vector<Item> items;

public:
    void add(std::shared_ptr<Course> c, std::string d, std::string t, std::string r) {
        Item item;
        item.course = c;
        item.day = d;
        item.time = t;
        item.room = r;
        items.push_back(item);
    }

    void print() {
        std::cout << "\n РАСПИСАНИЕ:\n";
        if (items.empty()) {
            std::cout << "Расписание пусто\n";
            return;
        }
        for (auto& i : items) {
            std::cout << i.day << " " << i.time << " ауд." << i.room
                << " - " << i.course->getName() << "\n";
        }
    }
};

//  ТЕСТИРОВАНИЕ
int main() {
    setlocale(LC_ALL, "Russian");

    try {
        //курсы
        auto math = std::make_shared<Lecture>("Математика", 30, "Интегралы");
        auto physics = std::make_shared<LabWork>("Физика", 20);
        physics->addTool("мультиметр");
        auto final = std::make_shared<Exam>("Высшая математика", 3, "05.03.2026");

        //оценки
        math->addGrade("Иванов", 95);
        math->addGrade("Петров", 87);
        math->addGrade("Сидоров", 92);

        physics->addGrade("Иванов", 88);
        physics->addGrade("Петров", 91);

        final->addGrade("Иванов", 98);
        final->addGrade("Петров", 84);
        final->addGrade("Сидоров", 89);

        //расписание
        Schedule schedule;
        schedule.add(math, "Пн", "10:00", "101");
        schedule.add(physics, "Ср", "14:00", "202");
        schedule.add(final, "Пт", "09:00", "Актовый зал");

        std::cout << " СИСТЕМА УПРАВЛЕНИЯ ОБРАЗОВАНИЕМ \n\n";

        math->start();
        physics->start();
        final->start();

        //расписание
        schedule.print();

        //статистика
        std::cout << "\n СТАТИСТИКА:\n";
        std::cout << "Средний балл по математике: " << math->getAverage() << "\n";

        int petrovGrade = physics->getGrade("Петров");
        if (petrovGrade != -1) {
            std::cout << "Оценка Петрова по физике: " << petrovGrade << "\n";
        }
        else {
            std::cout << "Петров не найден в списке по физике\n";
        }

        int smirnovGrade = physics->getGrade("Смирнов");
        if (smirnovGrade == -1) {
            std::cout << "Смирнов не найден в списке по физике\n";
        }

        //завершение занятий
        std::cout << "\n";
        math->finish();
        physics->finish();
        final->finish();

    }
    catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }

    return 0;
}