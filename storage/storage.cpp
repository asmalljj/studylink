#include "storage.h"
#include <fstream>
#include <sstream>
#include <cstdio>

// 极简 JSON 解析（只支持我们自己写出的固定格式）
static std::string get_json_string_field(const std::string& line, const std::string& key) {
    std::string pat = "\"" + key + "\":\"";
    auto pos = line.find(pat);
    if (pos == std::string::npos) return "";
    pos += pat.size();
    auto end = line.find('"', pos);
    if (end == std::string::npos) return "";
    return line.substr(pos, end - pos);
}

static long long get_json_ll_field(const std::string& line, const std::string& key) {
    std::string pat = "\"" + key + "\":";
    auto pos = line.find(pat);
    if (pos == std::string::npos) return 0;
    pos += pat.size();
    auto end = line.find_first_of(",}", pos);
    if (end == std::string::npos) return 0;
    return std::stoll(line.substr(pos, end - pos));
}

UserStorage::UserStorage(std::string users_file)
    : users_file_(std::move(users_file)) {}

std::string UserStorage::escape_json(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\\' || c == '"') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

bool UserStorage::parse_user_line(const std::string& line, UserRecord& out) {
    try {
        out.schema_version = (int)get_json_ll_field(line, "schema_version");
        out.uid = get_json_ll_field(line, "uid");
        out.username = get_json_string_field(line, "username");
        out.password_hash = get_json_string_field(line, "password_hash");
        out.created_at = get_json_string_field(line, "created_at");
        out.nickname = get_json_string_field(line, "nickname");

        // ✅ 学习信息
        out.goal = get_json_string_field(line, "goal");
        out.location = get_json_string_field(line, "location");
        out.time = get_json_string_field(line, "time");

        // 兼容老数据：没 nickname 就默认 username
        if (out.nickname.empty()) out.nickname = out.username;
        return !out.username.empty();
    } catch (...) {
        return false;
    }
}

bool UserStorage::append_user(const UserRecord& u, std::string& err) {
    std::ofstream ofs(users_file_, std::ios::app);
    if (!ofs) {
        err = "Failed to open users file for append: " + users_file_;
        return false;
    }

    ofs << "{"
        << "\"schema_version\":" << u.schema_version << ","
        << "\"uid\":" << u.uid << ","
        << "\"username\":\"" << escape_json(u.username) << "\","
        << "\"password_hash\":\"" << escape_json(u.password_hash) << "\","
        << "\"created_at\":\"" << escape_json(u.created_at) << "\","
        << "\"nickname\":\"" << escape_json(u.nickname) << "\","
        << "\"goal\":\"" << escape_json(u.goal) << "\","
        << "\"location\":\"" << escape_json(u.location) << "\","
        << "\"time\":\"" << escape_json(u.time) << "\""
        << "}\n";

    if (!ofs) {
        err = "Failed to write user record.";
        return false;
    }
    return true;
}

bool UserStorage::load_all(std::vector<UserRecord>& out, std::string& err) const {
    std::ifstream ifs(users_file_);
    if (!ifs) {
        out.clear();
        return true; // 文件不存在=空库
    }

    out.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        UserRecord u;
        if (!parse_user_line(line, u)) {
            err = "Failed to parse user line.";
            return false;
        }
        out.push_back(std::move(u));
    }
    return true;
}

std::optional<UserRecord> UserStorage::find_by_username(const std::string& username, std::string& err) const {
    std::ifstream ifs(users_file_);
    if (!ifs) return std::nullopt;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        UserRecord u;
        if (!parse_user_line(line, u)) {
            err = "Failed to parse user line.";
            return std::nullopt;
        }
        if (u.username == username) return u;
    }
    return std::nullopt;
}

bool UserStorage::update_user(const UserRecord& user, std::string& err) {
    std::vector<UserRecord> users;
    if (!load_all(users, err)) return false;

    bool found = false;
    for (auto& u : users) {
        if (u.username == user.username) {
            u = user;
            found = true;
            break;
        }
    }
    if (!found) {
        err = "User not found for update.";
        return false;
    }

    std::string tmp = users_file_ + ".tmp";
    std::ofstream ofs(tmp, std::ios::trunc);
    if (!ofs) {
        err = "Failed to open tmp file for rewrite: " + tmp;
        return false;
    }

    for (const auto& u : users) {
        ofs << "{"
            << "\"schema_version\":" << u.schema_version << ","
            << "\"uid\":" << u.uid << ","
            << "\"username\":\"" << escape_json(u.username) << "\","
            << "\"password_hash\":\"" << escape_json(u.password_hash) << "\","
            << "\"created_at\":\"" << escape_json(u.created_at) << "\","
            << "\"nickname\":\"" << escape_json(u.nickname) << "\","
            << "\"goal\":\"" << escape_json(u.goal) << "\","
            << "\"location\":\"" << escape_json(u.location) << "\","
            << "\"time\":\"" << escape_json(u.time) << "\""
            << "}\n";
    }

    ofs.close();
    if (!ofs) {
        err = "Failed while writing tmp file.";
        return false;
    }

    std::string bak = users_file_ + ".bak";
    std::remove(bak.c_str());
    std::rename(users_file_.c_str(), bak.c_str());
    if (std::rename(tmp.c_str(), users_file_.c_str()) != 0) {
        err = "Failed to replace users file.";
        return false;
    }
    
    return true;
}

bool UserStorage::delete_user(const std::string& username, std::string& err) {
    std::vector<UserRecord> users;
    if (!load_all(users, err)) return false;

    std::vector<UserRecord> kept;
    bool found = false;

    for (const auto& u : users) {
        if (u.username == username) {
            found = true;      // 发现要删的用户
            continue;          // 跳过 = 删除
        }
        kept.push_back(u);
    }

    if (!found) {
        err = "User not found.";
        return false;
    }

    // 重写文件：tmp -> 替换
    std::string tmp = users_file_ + ".tmp";
    std::ofstream ofs(tmp, std::ios::trunc);
    if (!ofs) {
        err = "Failed to open tmp file.";
        return false;
    }

    for (const auto& u : kept) {
        ofs << "{"
            << "\"schema_version\":" << u.schema_version << ","
            << "\"uid\":" << u.uid << ","
            << "\"username\":\"" << escape_json(u.username) << "\","
            << "\"password_hash\":\"" << escape_json(u.password_hash) << "\","
            << "\"created_at\":\"" << escape_json(u.created_at) << "\","
            << "\"nickname\":\"" << escape_json(u.nickname) << "\","
            << "\"goal\":\"" << escape_json(u.goal) << "\","
            << "\"location\":\"" << escape_json(u.location) << "\","
            << "\"time\":\"" << escape_json(u.time) << "\""
            << "}\n";
    }

    ofs.close();
    if (!ofs) {
        err = "Failed while writing tmp file.";
        return false;
    }

    std::string bak = users_file_ + ".bak";
    std::remove(bak.c_str());
    std::rename(users_file_.c_str(), bak.c_str()); // 旧文件可能不存在，忽略
    if (std::rename(tmp.c_str(), users_file_.c_str()) != 0) {
        err = "Failed to replace users file.";
        return false;
    }

    return true;
}
