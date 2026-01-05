#pragma once
#include <string>

bool migrate_users_v1_to_v2(const std::string& users_file, std::string& err);
//组员A：JSONL 存储、读取、更新、删除、迁移