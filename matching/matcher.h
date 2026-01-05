#pragma once
#include "../storage/storage.h"
#include "matching_config.h"
#include <vector>

struct MatchResult {
    UserRecord user;
    double total = 0.0;
    double s_goal = 0.0;
    double s_location = 0.0;
    double s_time = 0.0;
};

std::vector<MatchResult> recommend_users(
    const UserRecord& me,
    const std::vector<UserRecord>& all,
    const MatchingConfig& cfg
);
//组员B：匹配算法、策略切换、权重配置、推荐解释