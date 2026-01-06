#include "task.h"
#include "../team/team.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <limits>
#include <cctype>

static std::string now_date() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    return buf;
}

static std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace((unsigned char)s[i])) i++;
    while (j > i && std::isspace((unsigned char)s[j - 1])) j--;
    return s.substr(i, j - i);
}

static std::string extract_field(const std::string& line, const std::string& key) {
    auto pos = line.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = line.find(":", pos);
    if (pos == std::string::npos) return "";
    pos++;

    while (pos < line.size() && line[pos] == ' ') pos++;

    if (pos < line.size() && line[pos] == '"') {
        size_t end = line.find("\"", pos + 1);
        return line.substr(pos + 1, end - pos - 1);
    } else {
        size_t end = line.find_first_of(",}", pos);
        return line.substr(pos, end - pos);
    }
}

TaskStorage::TaskStorage(std::string file) : file_(std::move(file)) {}

std::string TaskStorage::escape_json(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

bool TaskStorage::parse_line(const std::string& line, TaskRecord& out) {
    if (line.empty()) return false;
    out.id = std::stoll(extract_field(line, "id"));
    out.owner = extract_field(line, "owner");
    out.team_id = std::stoll(extract_field(line, "team_id"));
    out.title = extract_field(line, "title");
    out.deadline = extract_field(line, "deadline");
    out.status = extract_field(line, "status");
    out.assignee = extract_field(line, "assignee");
    out.created_at = extract_field(line, "created_at");
    return true;
}

bool TaskStorage::append(const TaskRecord& t, std::string& err) {
    std::ofstream fout(file_, std::ios::app);
    if (!fout.is_open()) { err = "open tasks file failed"; return false; }

    fout << "{"
         << "\"id\":" << t.id << ","
         << "\"owner\":\"" << escape_json(t.owner) << "\","
         << "\"team_id\":" << t.team_id << ","
         << "\"title\":\"" << escape_json(t.title) << "\","
         << "\"deadline\":\"" << escape_json(t.deadline) << "\","
         << "\"status\":\"" << escape_json(t.status) << "\","
         << "\"assignee\":\"" << escape_json(t.assignee) << "\","
         << "\"created_at\":\"" << escape_json(t.created_at) << "\""
         << "}\n";
    return true;
}

bool TaskStorage::load_all(std::vector<TaskRecord>& out, std::string& err) const {
    std::ifstream fin(file_);
    if (!fin.is_open()) { err = "open tasks file failed"; return false; }

    std::string line;
    while (std::getline(fin, line)) {
        TaskRecord t;
        if (parse_line(line, t)) out.push_back(t);
    }
    return true;
}

std::optional<TaskRecord> TaskStorage::find_by_id(long long id, std::string& err) const {
    std::vector<TaskRecord> all;
    if (!load_all(all, err)) return std::nullopt;
    for (auto& t : all) if (t.id == id) return t;
    return std::nullopt;
}

long long TaskStorage::next_id(std::string& err) const {
    std::vector<TaskRecord> all;
    if (!load_all(all, err)) return 1;
    long long mx = 0;
    for (auto& t : all) mx = std::max(mx, t.id);
    return mx + 1;
}

bool TaskStorage::update(const TaskRecord& t, std::string& err) {
    std::vector<TaskRecord> all;
    if (!load_all(all, err)) return false;

    bool found = false;
    for (auto& x : all) {
        if (x.id == t.id) {
            x = t;
            found = true;
            break;
        }
    }
    if (!found) { err = "task id not found"; return false; }

    std::ofstream fout(file_, std::ios::trunc);
    if (!fout.is_open()) { err = "open tasks file failed"; return false; }

    for (auto& x : all) {
        append(x, err);
        if (!err.empty()) return false;
    }
    return true;
}

// ------------------ CLI Menus ------------------

static void print_task(const TaskRecord& t) {
    std::cout << "id=" << t.id
              << " status=" << t.status
              << " deadline=" << t.deadline
              << " title=" << t.title;
    if (t.team_id != 0) std::cout << " [assignee=" << t.assignee << "]";
    std::cout << "\n";
}

void my_study_tasks_menu(const std::string& username, const std::string& tasks_file, std::string& err) {
    err.clear();
    TaskStorage store(tasks_file);

    while (true) {
        std::cout << "\n=== My Study Tasks ===\n";
        std::cout << "1) View tasks\n";
        std::cout << "2) Add task\n";
        std::cout << "3) Update status\n";
        std::cout << "4) Back\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) return;
        if (op == 4) return;

        if (op == 1) {
            std::vector<TaskRecord> all;
            std::string e;
            if (!store.load_all(all, e)) { std::cout << "[ERR] " << e << "\n"; continue; }

            bool any = false;
            for (auto& t : all) {
                if (t.team_id == 0 && t.owner == username) {
                    print_task(t);
                    any = true;
                }
            }
            if (!any) std::cout << "(none)\n";
        }
        else if (op == 2) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            TaskRecord t;
            std::string title, deadline;

            std::cout << "title: ";
            std::getline(std::cin, title);
            std::cout << "deadline (e.g. 2026-01-10): ";
            std::getline(std::cin, deadline);

            t.id = store.next_id(err);
            t.owner = username;
            t.team_id = 0;
            t.title = trim(title);
            t.deadline = trim(deadline);
            t.status = "TODO";
            t.assignee = "";
            t.created_at = now_date();

            if (!store.append(t, err)) { std::cout << "[ERR] " << err << "\n"; continue; }
            std::cout << "[OK] Task added. id=" << t.id << "\n";
        }
        else if (op == 3) {
            std::cout << "task id: ";
            long long id; std::cin >> id;

            auto opt = store.find_by_id(id, err);
            if (!opt.has_value()) { std::cout << "[ERR] task not found\n"; continue; }

            auto t = *opt;
            if (!(t.team_id == 0 && t.owner == username)) {
                std::cout << "[ERR] not your personal task\n";
                continue;
            }

            std::cout << "new status (TODO/DOING/DONE): ";
            std::string st; std::cin >> st;
            if (st != "TODO" && st != "DOING" && st != "DONE") {
                std::cout << "[ERR] invalid status\n";
                continue;
            }
            t.status = st;

            if (!store.update(t, err)) { std::cout << "[ERR] " << err << "\n"; continue; }
            std::cout << "[OK] Updated.\n";
        }
    }
}

void team_study_plan_menu(const std::string& username,
                          const std::string& team_file,
                          const std::string& tasks_file,
                          std::string& err) {
    err.clear();

    TeamStorage tstore(team_file);
    auto teamOpt = tstore.find_by_username(username, err);
    if (!teamOpt.has_value()) {
        if (err.empty()) err = "you are not in any team";
        std::cout << "[ERR] " << err << "\n";
        return;
    }
    auto team = *teamOpt;

    TaskStorage store(tasks_file);

    while (true) {
        std::cout << "\n=== Team Study Plan (team_id=" << team.team_id << ") ===\n";
        std::cout << "1) View team tasks\n";
        std::cout << "2) Add team task\n";
        std::cout << "3) Update status\n";
        std::cout << "4) Back\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) return;
        if (op == 4) return;

        if (op == 1) {
            std::vector<TaskRecord> all;
            std::string e;
            if (!store.load_all(all, e)) { std::cout << "[ERR] " << e << "\n"; continue; }

            bool any = false;
            for (auto& t : all) {
                if (t.team_id == team.team_id) {
                    print_task(t);
                    any = true;
                }
            }
            if (!any) std::cout << "(none)\n";
        }
        else if (op == 2) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            TaskRecord t;
            std::string title, deadline, assignee;

            std::cout << "title: ";
            std::getline(std::cin, title);
            std::cout << "deadline (e.g. 2026-01-10): ";
            std::getline(std::cin, deadline);
            std::cout << "assignee (" << team.member1 << "/" << team.member2 << "): ";
            std::getline(std::cin, assignee);

            assignee = trim(assignee);
            if (assignee != team.member1 && assignee != team.member2) {
                std::cout << "[ERR] assignee must be a team member\n";
                continue;
            }

            t.id = store.next_id(err);
            t.owner = "";
            t.team_id = team.team_id;
            t.title = trim(title);
            t.deadline = trim(deadline);
            t.status = "TODO";
            t.assignee = assignee;
            t.created_at = now_date();

            if (!store.append(t, err)) { std::cout << "[ERR] " << err << "\n"; continue; }
            std::cout << "[OK] Team task added. id=" << t.id << "\n";
        }
        else if (op == 3) {
            std::cout << "task id: ";
            long long id; std::cin >> id;

            auto opt = store.find_by_id(id, err);
            if (!opt.has_value()) { std::cout << "[ERR] task not found\n"; continue; }

            auto t = *opt;
            if (t.team_id != team.team_id) {
                std::cout << "[ERR] not your team task\n";
                continue;
            }

            std::cout << "new status (TODO/DOING/DONE): ";
            std::string st; std::cin >> st;
            if (st != "TODO" && st != "DOING" && st != "DONE") {
                std::cout << "[ERR] invalid status\n";
                continue;
            }
            t.status = st;

            if (!store.update(t, err)) { std::cout << "[ERR] " << err << "\n"; continue; }
            std::cout << "[OK] Updated.\n";
        }
    }
}
