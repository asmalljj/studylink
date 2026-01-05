#pragma once
#include <string>

// 统计分析页：热门 goal、成功组队次数排行、平均评分排行
void analytics_dashboard(
    const std::string& user_file,
    const std::string& team_file,
    const std::string& review_file,
    std::string& err
);
