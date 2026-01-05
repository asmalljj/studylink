#pragma once
#include <string>

// 统计分析页：热门 goal、成功组队次数排行、平均评分排行
void analytics_dashboard(
    const std::string& user_file,
    const std::string& team_file,
    const std::string& review_file,
    std::string& err
);
//组员C：运营统计分析（热门goal、成功组队排行、评分排行）