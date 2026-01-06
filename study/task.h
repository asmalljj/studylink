#pragma once
#include <string>
#include <vector>
#include <optional>

struct TaskRecord {
    long long id = 0;
    std::string owner;       // personal task owner (username), empty if team task
    long long team_id = 0;   // 0 = personal task
    std::string title;
    std::string deadline;
    std::string status;      // TODO / DOING / DONE
    std::string assignee;    // only for team task
    std::string created_at;
};

class TaskStorage {
public:
    explicit TaskStorage(std::string file);

    bool append(const TaskRecord& t, std::string& err);
    bool load_all(std::vector<TaskRecord>& out, std::string& err) const;
    std::optional<TaskRecord> find_by_id(long long id, std::string& err) const;
    bool update(const TaskRecord& t, std::string& err);
    long long next_id(std::string& err) const;

private:
    std::string file_;
    static std::string escape_json(const std::string& s);
    static bool parse_line(const std::string& line, TaskRecord& out);
};

// ---- user-facing CLI functions ----
void my_study_tasks_menu(const std::string& username, const std::string& tasks_file, std::string& err);
void team_study_plan_menu(const std::string& username,
                          const std::string& team_file,
                          const std::string& tasks_file,
                          std::string& err);
