#include "word.h"
#include <memory>
#include <variant>
#include <vector>

#include "flow.h"
#include "timeline.h"

namespace gamedialog {
// =====
// public
// =====
void Timeline::goto_begin() {
  stages[current_]->clean();
  current_ = 0;
}
void Timeline::goto_end() {
  stages[current_]->clean();
  current_ = stages.size();
}
void Timeline::skip_stage_count(int count) {
  stages[current_]->clean();
  current_ += count;
  if (current_ > stages.size() - 1) {
    current_ = stages.size();
  }
}
Timeline::Timeline(const std::string &data) {
  std::istringstream stream(data);
  std::string line;

  std::string tt = "";
  std::vector<std::string> cur_secions;
  while (std::getline(stream, line)) {
    if (line == "") {
      continue;
    }
    // 处理场景标记 [scene]
    if (line[0] == '[' && line.back() == ']') {
      if (!cur_secions.empty() && tt != "") {
        // 解析stage
        auto cur = std::make_shared<DiaStage>(cur_secions);
        stages.push_back(cur);
        cur->set_timeline(this);
        stage_map[cur->get_stage_name()] = stages.size() - 1;
        cur_secions.clear();
        tt = "";
      }
      tt = line.substr(1, line.length() - 2);
    }
    cur_secions.push_back(line);
  }

  // 处理最后一段对话
  if (!cur_secions.empty()) {
    // _parse_section(cur_stage, cur_names, cur_word);
    // 解析stage
    auto cur = std::make_shared<DiaStage>(cur_secions);
    stages.push_back(cur);
    cur->set_timeline(this);
    stage_map[cur->get_stage_name()] = stages.size() - 1;
  }
}

std::string Timeline::current_stage() {
  if (current_ >= stages.size()) {
    return "";
  }
  auto cur = stages[current_];
  if (cur != nullptr) {
    return cur->get_stage_name();
  }
  return "";
}

bool Timeline::has_next() const {
  if (current_ >= stages.size()) {
    return false;
  }
  // 判断当前如果是标签，如果是:end那么就没有了
  return stages[current_]->has_next();
}

void Timeline::check_flag() {
  if (fn == nullptr) {
    return;
  }
  // 找到第一个满足flag的stage
  for (auto item : stages) {
    auto flags = item->get_flags();
    bool isok = true;
    for (auto flag : flags) {
      if (!fn(flag)) {
        isok = false;
        break;
      }
    }
    if (isok) {
      goto_stage(item->get_stage_name());
      return;
    }
  }
  // 没有一个满足条件的
  goto_end();
}

std::shared_ptr<DialogueWord> Timeline::next() {
  check_flag();
  if (!has_next()) {
    return nullptr;
  }
  return stages[current_]->next();
}

std::vector<std::string> Timeline::all_stages() {
  std::vector<std::string> ret;
  for (auto item : stages) {
    ret.push_back(item->get_stage_name());
  }
  return ret;
}

void Timeline::goto_stage(const std::string &stage) {
  if (stage_map.find(stage) == stage_map.end()) {
    return;
  }
  stages[current_]->clean();
  current_ = stage_map[stage];
}

int Timeline::stage_index(const std::string &label) {
  if (stage_map.find(label) == stage_map.end()) {
    return -1;
  }
  return stage_map[label];
}
} // namespace gamedialog