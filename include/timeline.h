#pragma once

#include "word.h"
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
namespace gamedialog {
class ControlFlow;
class Timeline {
public:
  using LineVariant =
      std::variant<std::shared_ptr<DialogueWord>, std::shared_ptr<ControlFlow>>;

private:
  std::vector<LineVariant> dialogue_keys;
  int current_ = 0;
  std::map<std::string, int> stage_map;
  std::vector<std::string> stage_list;

public:
  Timeline() = delete;
  Timeline(const std::string &data);

public:
  std::shared_ptr<DialogueWord> next();
  bool has_next() const;
  void goto_stage(const std::string &stage);
  void goto_begin() { current_ = 0; }
  void goto_end() { current_ = dialogue_keys.size(); }
  void skip_stage_count(int count) {
    // 寻找当前stage在stage_list中的位置,
    // 然后加count，超过则置为stage_list.size()，然后从stage_map找到对应的下标然后更新current_
    int index =
        std::find(stage_list.begin(), stage_list.end(), current_stage()) -
        stage_list.begin();
    index += count;
    if (index > stage_list.size() - 1) {
      index = stage_list.size() - 1;
    }
    current_ = stage_map[stage_list[index]];
  }
  std::vector<std::string> all_stages();
  bool has_stage(const std::string &name) const {
    return stage_map.find(name) != stage_map.end();
  }
  int stage_index(const std::string &label);
  std::string current_stage();

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

// 基础控制流类
class ControlFlow {
public:
  std::string stage_name;

public:
  virtual ~ControlFlow() = default;
  virtual void execute(Timeline *timeline) = 0;
  virtual bool hasNext(const Timeline &timeline) const = 0;
  virtual std::string getName() const = 0;

public:
  void set_stage_name(const std::string &name) { stage_name = name; }
  const std::string &get_stage_name() const { return stage_name; }
};

// Start分支 - 回到开头
class StartFlow : public ControlFlow {
public:
  void execute(Timeline *timeline_) override {
    // 实现回到开头的逻辑
    if (timeline_) {
      timeline_->goto_begin();
    }
  }

  std::string getName() const override { return "start"; }
  bool hasNext(const Timeline &timeline) const override { return true; }
};

// End分支 - 退出
class EndFlow : public ControlFlow {
public:
  void execute(Timeline *timeline_) override {
    if (timeline_) {
      timeline_->goto_end();
    }
  }

  std::string getName() const override { return "end"; }
  bool hasNext(const Timeline &timeline) const override { return false; }
};

// Skip分支 - 跳过n个stage
class SkipFlow : public ControlFlow {
private:
  int skipCount;

public:
  explicit SkipFlow(int count) : skipCount(count) {}

  void execute(Timeline *timeline_) override {
    if (timeline_) {
      timeline_->skip_stage_count(skipCount);
    }
  }

  std::string getName() const override { return "skip"; }
  bool hasNext(const Timeline &timeline) const override { return true; }

  int getSkipCount() const { return skipCount; }
};

// Goto分支 - 跳转到指定stage
class GotoFlow : public ControlFlow {
private:
  std::string targetName;

public:
  explicit GotoFlow(const std::string &name) : targetName(name) {}

  void execute(Timeline *timeline_) override {
    if (timeline_) {
      timeline_->goto_stage(targetName);
    }
  }

  std::string getName() const override { return "goto"; }
  bool hasNext(const Timeline &timeline) const override { 
    return timeline.has_stage(targetName);
   }
  std::string getTargetName() const { return targetName; }
};

// 控制流工厂类
class ControlFlowFactory {
public:
  static std::shared_ptr<ControlFlow>
  createFromString(const std::string &command) {
    // 解析命令字符串
    if (command == ":start") {
      return std::make_shared<StartFlow>();
    } else if (command == ":end") {
      return std::make_shared<EndFlow>();
    } else if (command.substr(0, 6) == ":skip:") {
      int count = std::stoi(command.substr(6));
      return std::make_shared<SkipFlow>(count);
    } else if (command.substr(0, 6) == ":goto:") {
      std::string target = command.substr(6);
      return std::make_shared<GotoFlow>(target);
    } else {
      return nullptr;
    }
  }
};
} // namespace gamedialog