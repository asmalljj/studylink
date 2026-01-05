#pragma once
#include <string>

struct MatchingConfig {
    double w_goal = 0.4;
    double w_location = 0.3;
    double w_time = 0.3;

    std::string strategy = "weighted"; // weighted | rule
    int topN = 5;
};

bool load_matching_config(const std::string& path, MatchingConfig& cfg);
bool save_matching_config(const std::string& path, const MatchingConfig& cfg);
//组员B：匹配算法、策略切换、权重配置、推荐解释