#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

//статусы задач
enum Status { BACKLOG, TODO, PROGRESS, DONE };
//приоритеты
enum Priority { LOW, MEDIUM, HIGH };

//преобразование статуса в строку
std::string statusStr(Status s) {
    if (s == BACKLOG) return "BACKLOG";
    if (s == TODO) return "TODO";
    if (s == PROGRESS) return "IN_PROGRESS";
    return "DONE";
}

//преобразование приоритета в строку
std::string priorityStr(Priority p) {
    if (p == LOW) return "LOW";
    if (p == MEDIUM) return "MEDIUM";
    return "HIGH";
}

// ========== БАЗОВЫЙ КЛАСС ProjectItem ==========
class ProjectItem {
protected:
    std::string title;
    std::string description;
    Status status;
    Priority priority;
    std::vector<std::shared_ptr<ProjectItem>> dependencies;

public:
    ProjectItem(std::string t, std::string d)
        : title(t), description(d), status(BACKLOG), priority(MEDIUM) {}

    virtual ~ProjectItem() = default;

    //виртуальный метод для типа элемента
    virtual std::string getType() = 0;

    //обновление статуса с проверкой зависимостей
    void setStatus(Status s) {
        //проверка зависимостей перед завершением
        if (s == DONE) {
            for (auto& dep : dependencies) {
                if (dep->getStatus() != DONE) {
                    std::cout << "Cannot complete: dependency "
                        << dep->getTitle() << " is not done\n";
                    return;
                }
            }
        }
        status = s;
        std::cout << title << " status changed to " << statusStr(s) << "\n";
    }

    //добавление зависимости
    void addDependency(std::shared_ptr<ProjectItem> item) {
        dependencies.push_back(item);
        std::cout << "Dependency added: " << title << " -> " << item->getTitle() << "\n";
    }

    std::string getTitle() { return title; }
    Status getStatus() { return status; }
    Priority getPriority() { return priority; }

    void setPriority(Priority p) { priority = p; }

    //вывод информации
    void display() {
        std::cout << "[" << getType() << "] " << title << "\n";
        std::cout << "  Status: " << statusStr(status) << "\n";
        std::cout << "  Priority: " << priorityStr(priority) << "\n";
        std::cout << "  Description: " << description << "\n";

        if (!dependencies.empty()) {
            std::cout << "  Depends on: ";
            for (auto& dep : dependencies) {
                std::cout << dep->getTitle() << " ";
            }
            std::cout << "\n";
        }
    }
};

//КЛАСС TASK
class Task : public ProjectItem {
public:
    Task(std::string t, std::string d) : ProjectItem(t, d) {}
    std::string getType() override { return "TASK"; }
};

//КЛАСС BUG
class Bug : public ProjectItem {
public:
    Bug(std::string t, std::string d) : ProjectItem(t, d) {}
    std::string getType() override { return "BUG"; }
};

//КЛАСС MILESTONE
class Milestone : public ProjectItem {
private:
    std::vector<std::shared_ptr<ProjectItem>> items;

public:
    Milestone(std::string t, std::string d) : ProjectItem(t, d) {}
    std::string getType() override { return "MILESTONE"; }

    void addItem(std::shared_ptr<ProjectItem> item) {
        items.push_back(item);
    }

    int getProgress() {
        if (items.empty()) return 0;
        int done = 0;
        for (auto& item : items) {
            if (item->getStatus() == DONE) done++;
        }
        return (done * 100) / items.size();
    }

    //переопределенный вывод
    void display() {
        ProjectItem::display();
        std::cout << "  Progress: " << getProgress() << "%\n";
        std::cout << "  Items in milestone: " << items.size() << "\n";
    }
};

//КЛАСС PROJECT
class Project : public ProjectItem {
private:
    std::vector<std::shared_ptr<ProjectItem>> items; //все элементы проекта

public:
    Project(std::string t, std::string d) : ProjectItem(t, d) {}
    std::string getType() override { return "PROJECT"; }

    //добавление элемента в проект
    void addItem(std::shared_ptr<ProjectItem> item) {
        items.push_back(item);
        std::cout << "Added " << item->getType() << ": " << item->getTitle() << "\n";
    }

    //поиск элемента по названию
    std::shared_ptr<ProjectItem> findItem(std::string title) {
        for (auto& item : items) {
            if (item->getTitle() == title) return item;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<ProjectItem>> getByStatus(Status s) {
        std::vector<std::shared_ptr<ProjectItem>> result;
        for (auto& item : items) {
            if (item->getStatus() == s) result.push_back(item);
        }
        return result;
    }

    std::vector<std::shared_ptr<ProjectItem>> getByPriority(Priority p) {
        std::vector<std::shared_ptr<ProjectItem>> result;
        for (auto& item : items) {
            if (item->getPriority() == p) result.push_back(item);
        }
        return result;
    }

    //завершение проекта 
    void complete() {
        bool allDone = true;
        for (auto& item : items) {
            if (item->getStatus() != DONE) {
                allDone = false;
                std::cout << "Not completed: " << item->getTitle() << "\n";
            }
        }

        if (allDone) {
            setStatus(DONE);
            std::cout << "Project " << title << " completed successfully!\n";
        }
        else {
            std::cout << "Cannot complete project: some items are not done\n";
        }
    }

    //вывод всех элементов
    void displayAll() {
        std::cout << "\n=== PROJECT: " << title << " ===\n";
        ProjectItem::display();
        std::cout << "\nProject items (" << items.size() << "):\n";
        for (auto& item : items) {
            std::cout << "  - " << item->getType() << ": " << item->getTitle()
                << " [" << statusStr(item->getStatus()) << "]\n";
        }
    }
};

//КЛАСС ProjectManager ДЛЯ УПРАВЛЕНИЯ
class ProjectManager {
private:
    std::vector<std::shared_ptr<Project>> projects;

public:
    //добавление проекта
    void addProject(std::shared_ptr<Project> p) {
        projects.push_back(p);
        std::cout << "Project added: " << p->getTitle() << "\n";
    }

    //поиск проекта
    std::shared_ptr<Project> findProject(std::string title) {
        for (auto& p : projects) {
            if (p->getTitle() == title) return p;
        }
        return nullptr;
    }

    //удаление проекта
    void removeProject(std::string title) {
        auto it = std::remove_if(projects.begin(), projects.end(),
            [title](std::shared_ptr<Project> p) { return p->getTitle() == title; });
        if (it != projects.end()) {
            projects.erase(it, projects.end());
            std::cout << "Project removed: " << title << "\n";
        }
    }

    //вывод всех проектов
    void displayAll() {
        std::cout << "\n=== ALL PROJECTS (" << projects.size() << ") ===\n";
        for (auto& p : projects) {
            std::cout << "  - " << p->getTitle() << "\n";
        }
    }
};

int main() {
    //создание менеджера проектов
    ProjectManager pm;

    //создание проекта
    auto project = std::make_shared<Project>("WebApp", "Develop web application");
    pm.addProject(project);

    //создание задач
    auto task1 = std::make_shared<Task>("Design", "Create UI design");
    auto task2 = std::make_shared<Task>("Frontend", "Implement frontend");
    auto task3 = std::make_shared<Task>("Backend", "Implement backend");
    auto bug1 = std::make_shared<Bug>("Login bug", "Fix login issue");
    auto milestone = std::make_shared<Milestone>("Release v1", "First release");

    //установка приоритетов
    task1->setPriority(HIGH);
    bug1->setPriority(HIGH);

    //добавление зависимостей
    task2->addDependency(task1);
    task3->addDependency(task2);
    bug1->addDependency(task3);

    //добавление задач
    milestone->addItem(task1);
    milestone->addItem(task2);
    milestone->addItem(task3);
    milestone->addItem(bug1);

    //добавление элементов в проект
    project->addItem(task1);
    project->addItem(task2);
    project->addItem(task3);
    project->addItem(bug1);
    project->addItem(milestone);

    std::cout << "\n=== DEMO ===\n";

    task2->setStatus(DONE);

    task1->setStatus(DONE);
    task2->setStatus(DONE);
    task3->setStatus(DONE);

    bug1->setStatus(DONE);

    //завершение проекта
    project->complete();

    //вывод информации
    task1->display();
    milestone->display();
    project->displayAll();

    //поиск по статусу
    std::cout << "\n=== DONE ITEMS ===\n";
    auto doneItems = project->getByStatus(DONE);
    for (auto& item : doneItems) {
        std::cout << "  - " << item->getTitle() << "\n";
    }

    return 0;
}