#pragma once
#include <string>

struct Meta {
    int version = 1;
};

class MetaStorage {
public:
    explicit MetaStorage(std::string meta_file);

    // 如果不存在则创建默认 meta
    bool load_or_init(Meta& out, std::string& err);

    bool save(const Meta& meta, std::string& err);

private:
    std::string meta_file_;
};
//组员A：JSONL 存储、读取、更新、删除、迁移