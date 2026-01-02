#pragma once
#include <string>

bool migrate_users_v1_to_v2(const std::string& users_file, std::string& err);
