#pragma once

#include "stage.h"
#include "word.h"
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace gamedialog {
class ControlFlow;
class DiaStage;
class Timeline {
public:
  using precheckfn = std::function<bool(const std::string &expr)>;

private:
  std::vector<std::shared_ptr<DiaStage>> stages;
  int current_ = 0;
  std::map<std::string, int> stage_map;
  precheckfn fn;

public:
  Timeline() = delete;
  Timeline(const std::string &data);
  void set_precheck(precheckfn fn) { this->fn = fn; }

public:
  std::shared_ptr<DialogueWord> next();
  bool has_next() const;
  void goto_stage(const std::string &stage);
  void goto_begin();
  void goto_end();
  void skip_stage_count(int count);
  void check_flag();
  std::vector<std::string> all_stages();
  bool has_stage(const std::string &name) const {
    return stage_map.find(name) != stage_map.end();
  }
  int stage_index(const std::string &label);
  std::string current_stage();
};

} // namespace gamedialog