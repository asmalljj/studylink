#include "analytics.h"
#include "../storage/storage.h"
#include "../team/team.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>

// team.cpp 里用的解析工具（复制一份，避免依赖 team.cpp 内部 static）
static std::string analytics_extract_field(const std::string& line, const std::string& key) {
    auto pos = line.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = line.find(":", pos);
    if (pos == std::string::npos) return "";
    pos++;

    while (pos < line.size() && (line[pos] == ' ')) pos++;

    if (pos < line.size() && line[pos] == '"') {
        size_t end = line.find("\"", pos + 1);
        return line.substr(pos + 1, end - pos - 1);
    } else {
        size_t end = line.find_first_of(",}", pos);
        return line.substr(pos, end - pos);
    }
}

struct ReviewLine {
    long long team_id = 0;
    std::string from_user;
    std::string to_user;
    int score = 0;
};

static bool parse_review_line(const std::string& line, ReviewLine& out) {
    if (line.empty()) return false;
    out.team_id = std::stoll(analytics_extract_field(line, "team_id"));
    out.from_user = analytics_extract_field(line, "from");
    out.to_user = analytics_extract_field(line, "to");
    out.score = std::stoi(analytics_extract_field(line, "score"));
    return true;
}

void analytics_dashboard(
    const std::string& user_file,
    const std::string& team_file,
    const std::string& review_file,
    std::string& err
) {
    err.clear();
    std::cout << "=== Analytics Dashboard ===\n";

    // 1) Popular Goals
    {
        UserStorage us(user_file);
        std::vector<UserRecord> users;
        std::string uerr;
        if (us.load_all(users, uerr)) {
            std::unordered_map<std::string, int> cnt;
            for (auto& u : users) {
                if (!u.goal.empty()) cnt[u.goal]++;
            }
            std::vector<std::pair<std::string,int>> v(cnt.begin(), cnt.end());
            std::sort(v.begin(), v.end(), [](auto& a, auto& b){ return a.second > b.second; });

            std::cout << "\n[Popular Goals]\n";
            if (v.empty()) std::cout << "(no goal data)\n";
            for (size_t i=0; i<v.size() && i<10; ++i) {
                std::cout << (i+1) << ". " << v[i].first << " (" << v[i].second << " users)\n";
            }
        } else {
            std::cout << "\n[Popular Goals]\n";
            std::cout << "(cannot load users: " << uerr << ")\n";
        }
    }

    // 2) Successful Teaming Rank
    {
        TeamStorage ts(team_file);
        std::vector<TeamRecord> teams;
        std::string terr;
        if (ts.load_all(teams, terr)) {
            std::unordered_map<std::string, int> cnt;
            for (auto& t : teams) {
                cnt[t.member1]++;
                cnt[t.member2]++;
            }
            std::vector<std::pair<std::string,int>> v(cnt.begin(), cnt.end());
            std::sort(v.begin(), v.end(), [](auto& a, auto& b){ return a.second > b.second; });

            std::cout << "\n[Successful Teaming Rank]\n";
            if (v.empty()) std::cout << "(no team data)\n";
            for (size_t i=0; i<v.size() && i<10; ++i) {
                std::cout << (i+1) << ". " << v[i].first << " (teams=" << v[i].second << ")\n";
            }
        } else {
            std::cout << "\n[Successful Teaming Rank]\n";
            std::cout << "(cannot load teams: " << terr << ")\n";
        }
    }

    // 3) Average Rating Rank
    {
        std::ifstream fin(review_file);
        std::unordered_map<std::string, std::pair<int,int>> agg; // user -> (sum, count)

        if (fin.is_open()) {
            std::string line;
            while (std::getline(fin, line)) {
                ReviewLine r;
                if (!parse_review_line(line, r)) continue;
                auto& p = agg[r.to_user];
                p.first += r.score;
                p.second += 1;
            }

            std::vector<std::pair<std::string, double>> v;
            for (auto& kv : agg) {
                if (kv.second.second == 0) continue;
                v.push_back({kv.first, (double)kv.second.first / kv.second.second});
            }
            std::sort(v.begin(), v.end(), [](auto& a, auto& b){ return a.second > b.second; });

            std::cout << "\n[Average Rating Rank]\n";
            if (v.empty()) std::cout << "(no review data)\n";
            for (size_t i=0; i<v.size() && i<10; ++i) {
                std::cout << (i+1) << ". " << v[i].first << " (avg=" << v[i].second << ")\n";
            }
        } else {
            std::cout << "\n[Average Rating Rank]\n";
            std::cout << "(cannot open reviews.jsonl)\n";
        }
    }

    std::cout << "\n==========================\n";
}
