#include "migrate.h"
#include "storage.h"
#include <fstream>
#include <vector>
#include <cstdio>   // std::remove, std::rename

bool migrate_users_v1_to_v2(const std::string& users_file, std::string& err) {
    UserStorage st(users_file);

    std::vector<UserRecord> users;
    if (!st.load_all(users, err)) return false;

    // 生成新文件
    std::string tmp = users_file + ".tmp";
    std::ofstream ofs(tmp, std::ios::trunc);
    if (!ofs) {
        err = "Failed to create tmp file: " + tmp;
        return false;
    }

    long long next_uid = 1;
    for (auto& u : users) {
        // 老记录补齐 v2 字段
        u.schema_version = 2;
        if (u.uid == 0) u.uid = next_uid++;
        if (u.nickname.empty()) u.nickname = u.username;

        // 直接写 jsonl（复用 escape_json 简单点：这里用 UserStorage::append_user 不方便，因为它追加打开）
        // 为了简单，这里手写一份输出格式：
        auto esc = [](const std::string& s) {
            std::string out;
            for (char c : s) {
                if (c == '\\' || c == '"') out.push_back('\\');
                out.push_back(c);
            }
            return out;
        };

        ofs << "{"
            << "\"schema_version\":" << u.schema_version << ","
            << "\"uid\":" << u.uid << ","
            << "\"username\":\"" << esc(u.username) << "\","
            << "\"password_hash\":\"" << esc(u.password_hash) << "\","
            << "\"created_at\":\"" << esc(u.created_at) << "\","
            << "\"nickname\":\"" << esc(u.nickname) << "\""
            << "}\n";
    }

    ofs.close();
    if (!ofs) {
        err = "Failed writing tmp file.";
        return false;
    }

    // 用 tmp 替换旧文件
    std::string bak = users_file + ".bak";
    std::remove(bak.c_str());
    if (std::rename(users_file.c_str(), bak.c_str()) != 0) {
        // 如果旧文件不存在（空库），也没关系：直接把 tmp 改名成 users_file
    }
    if (std::rename(tmp.c_str(), users_file.c_str()) != 0) {
        err = "Failed to replace users file.";
        return false;
    }
    return true;
}
