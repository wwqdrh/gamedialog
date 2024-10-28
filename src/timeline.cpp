#include "timeline.h"
#include "word.h"
#include <memory>

namespace gamedialog {
// =====
// public
// =====
Timeline::Timeline(const std::string &data) {
  std::vector<std::string> cur_names;
  std::vector<std::string> cur_word;
  std::istringstream stream(data);
  std::string line;

  std::string cur_stage = "";
  while (std::getline(stream, line)) {
    // 跳过注释和空行
    if (line[0] == '#' || is_empty(strip(line))) {
      continue;
    }

    // 处理场景标记 [scene]
    if (line[0] == '[' && line.back() == ']') {
      if (!cur_names.empty() && !cur_word.empty()) {
        _parse_section(cur_stage, cur_names, cur_word);
        cur_names.clear();
        cur_word.clear();
      }
      cur_stage = line.substr(1, line.length() - 2);
      stage_map[cur_stage] = dialogue_keys.size();
    }
    // 处理角色名称 (name1,name2)
    else if (line[0] == '(' && line.back() == ')') {
      if (!cur_names.empty() && !cur_word.empty()) {
        _parse_section(cur_stage, cur_names, cur_word);
        cur_names.clear();
        cur_word.clear();
      }
      std::string names_str = line.substr(1, line.length() - 2);
      cur_names = split(names_str, ',');
    }
    // 处理对话内容
    else {
      cur_word.push_back(line);
    }
  }

  // 处理最后一段对话
  if (!cur_names.empty() && !cur_word.empty()) {
    _parse_section(cur_stage, cur_names, cur_word);
  }
}

std::shared_ptr<DialogueWord> Timeline::next() {
  if (!has_next()) {
    return nullptr;
  }
  std::shared_ptr<DialogueWord> ret = dialogue_keys[current_];
  current_++;
  return ret;
}

std::vector<std::string> Timeline::all_stages() {
  std::vector<std::string> ret;
  for (const auto &pair : stage_map) {
    ret.push_back(pair.first);
  }
  return ret;
}

void Timeline::goto_stage(const std::string &stage) {
  if (stage_map.find(stage) == stage_map.end()) {
    return;
  }
  current_ = stage_map[stage];
}

int Timeline::stage_index(const std::string &label) {
  if (stage_map.find(label) == stage_map.end()) {
    return -1;
  }
  return stage_map[label];
}

// =====
// private
// ====
void Timeline::_parse_section(const std::string stage_name,
                              const std::vector<std::string> &names,
                              const std::vector<std::string> &words) {
  int curindex = 0;
  std::shared_ptr<DialogueWord> cur = nullptr;

  for (const auto &word : words) {
    // 跳过注释和空行
    if (starts_with(word, "#") || word.empty()) {
      continue;
    }

    // 处理响应选项
    if (starts_with(word, "-")) {
      if (cur != nullptr) {
        std::string response = trim_prefix(word, "-");
        auto parts = split(response, ':');
        if (parts.size() == 2) {
          cur->add_response(parts[0], parts[1]);
        }
      }
    }
    // 处理函数调用
    else if (starts_with(word, "@")) {
      if (cur != nullptr) {
        cur->add_fn(trim_prefix(word, "@"));
      }
    }
    // 处理普通对话
    else {
      cur = std::make_shared<DialogueWord>();
      cur->set_stage(stage_name);
      cur->set_name(names[curindex]);
      cur->set_text(trim_suffix(word, "+"));
      dialogue_keys.push_back(cur);

      if (!ends_with(word, "+")) {
        curindex = (curindex + 1) % names.size();
      }
    }
  }
}
} // namespace gamedialog