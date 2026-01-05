#include "meta.h"
#include <fstream>
#include <sstream>

MetaStorage::MetaStorage(std::string meta_file)
    : meta_file_(std::move(meta_file)) {}

bool MetaStorage::load_or_init(Meta& out, std::string& err) {
    std::ifstream ifs(meta_file_);
    if (!ifs) {
        // 不存在就初始化
        out.version = 1;
        return save(out, err);
    }

    // 超简解析：只识别 {"version":2}
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string s = ss.str();

    auto pos = s.find("\"version\":");
    if (pos == std::string::npos) {
        err = "meta.json missing version field";
        return false;
    }
    pos += std::string("\"version\":").size();
    out.version = std::stoi(s.substr(pos));
    return true;
}

bool MetaStorage::save(const Meta& meta, std::string& err) {
    std::ofstream ofs(meta_file_, std::ios::trunc);
    if (!ofs) {
        err = "Failed to write meta file: " + meta_file_;
        return false;
    }
    ofs << "{ \"version\": " << meta.version << " }\n";
    return true;
}
//组员A：JSONL 存储、读取、更新、删除、迁移