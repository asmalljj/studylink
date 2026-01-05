#include "matching_config.h"
#include <fstream>
#include <sstream>

static std::string read_all(const std::string& path) {
    std::ifstream fin(path);
    if (!fin.is_open()) return "";
    std::ostringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

static std::string get_json_value(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n')) pos++;
    size_t end = pos;
    while (end < json.size() && json[end] != ',' && json[end] != '\n' && json[end] != '}') end++;
    return json.substr(pos, end - pos);
}

bool load_matching_config(const std::string& path, MatchingConfig& cfg) {
    std::string json = read_all(path);
    if (json.empty()) return false;

    auto v1 = get_json_value(json, "w_goal");
    auto v2 = get_json_value(json, "w_location");
    auto v3 = get_json_value(json, "w_time");
    auto vs = get_json_value(json, "strategy");
    auto vt = get_json_value(json, "topN");

    if (!v1.empty()) cfg.w_goal = std::stod(v1);
    if (!v2.empty()) cfg.w_location = std::stod(v2);
    if (!v3.empty()) cfg.w_time = std::stod(v3);
    if (!vt.empty()) cfg.topN = std::stoi(vt);

    if (!vs.empty()) {
        // remove quotes
        if (vs.front() == '"') vs = vs.substr(1, vs.size() - 2);
        cfg.strategy = vs;
    }
    return true;
}

bool save_matching_config(const std::string& path, const MatchingConfig& cfg) {
    std::ofstream fout(path);
    if (!fout.is_open()) return false;
    fout << "{\n";
    fout << "  \"w_goal\": " << cfg.w_goal << ",\n";
    fout << "  \"w_location\": " << cfg.w_location << ",\n";
    fout << "  \"w_time\": " << cfg.w_time << ",\n";
    fout << "  \"strategy\": \"" << cfg.strategy << "\",\n";
    fout << "  \"topN\": " << cfg.topN << "\n";
    fout << "}\n";
    return true;
}
