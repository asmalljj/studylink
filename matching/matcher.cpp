#include "matcher.h"
#include <algorithm>

static double similarity(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty()) return 0.0;
    if (a == b) return 1.0;

    // 简易文本匹配：包含关系给予较高分
    if (a.find(b) != std::string::npos || b.find(a) != std::string::npos)
        return 0.7;

    return 0.2;
}

static bool rule_match(const UserRecord& me, const UserRecord& other) {
    // Rule-based：同地点 + 同时段
    return me.location == other.location && me.time == other.time;
}

std::vector<MatchResult> recommend_users(
    const UserRecord& me,
    const std::vector<UserRecord>& all,
    const MatchingConfig& cfg
) {
    std::vector<MatchResult> res;

    for (auto& u : all) {
        if (u.username == me.username) continue;

        if (cfg.strategy == "rule" && !rule_match(me, u))
            continue;

        double sg = similarity(me.goal, u.goal);
        double sl = similarity(me.location, u.location);
        double st = similarity(me.time, u.time);

        double total = cfg.w_goal * sg + cfg.w_location * sl + cfg.w_time * st;
        res.push_back({u, total, sg, sl, st});
    }

    std::sort(res.begin(), res.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.total > b.total;
    });

    if ((int)res.size() > cfg.topN) res.resize(cfg.topN);
    return res;
}
//组员B：匹配算法、策略切换、权重配置、推荐解释