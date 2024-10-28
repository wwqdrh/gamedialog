#pragma once

#include "word.h"
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
namespace gamedialog {
class Timeline {
private:
  std::vector<std::shared_ptr<DialogueWord>> dialogue_keys;
  int current_ = 0;
  std::map<std::string, int> stage_map;

public:
  Timeline() = delete;
  Timeline(const std::string &data);

public:
  std::shared_ptr<DialogueWord> next();
  bool has_next() const { return current_ < dialogue_keys.size(); }
  void goto_stage(const std::string &stage);
  void goto_begin() { current_ = 0; }
  std::vector<std::string> all_stages();
  int stage_index(const std::string &label);
  std::string current_stage() {
    if (current_ < dialogue_keys.size()) {
      return dialogue_keys[current_]->get_stage();
    }
    return "";
  }

private:
  void _parse_section(const std::string stage_name,
                      const std::vector<std::string> &names,
                      const std::vector<std::string> &words);
  // 检查字符串是否为空或只包含空白字符
  bool is_empty(const std::string &str) const {
    return str.find_first_not_of(" \t\n\r") == std::string::npos;
  }
  // 分割字符串
  std::vector<std::string> split(const std::string &str, char delimiter) const {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, delimiter)) {
      tokens.push_back(token);
    }
    return tokens;
  }
  // 去除字符串两端的空白字符
  std::string strip(const std::string &str) const {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
      return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
  }
  // 辅助函数：去除前缀
  std::string trim_prefix(const std::string &str,
                          const std::string &prefix) const {
    if (starts_with(str, prefix)) {
      return str.substr(prefix.length());
    }
    return str;
  }

  // 辅助函数：去除后缀
  std::string trim_suffix(const std::string &str,
                          const std::string &suffix) const {
    if (ends_with(str, suffix)) {
      return str.substr(0, str.length() - suffix.length());
    }
    return str;
  }
  // 辅助函数：检查字符串是否以特定前缀开始
  bool starts_with(const std::string &str, const std::string &prefix) const {
    return str.size() >= prefix.size() &&
           str.compare(0, prefix.size(), prefix) == 0;
  }

  // 辅助函数：检查字符串是否以特定后缀结束
  bool ends_with(const std::string &str, const std::string &suffix) const {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  }
};
} // namespace gamedialog