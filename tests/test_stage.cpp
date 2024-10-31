#include "stage.h"
#include <gtest/gtest.h>

#include "stage.h"

TEST(StageTest, TestParse) {
  gamedialog::DiaStage stage(R"(
[stage1]
(John,Mary)
Hello there!
-Yes: next_scene
-No: end_scene
Yes, Hello!
    )");

  ASSERT_EQ(stage.get_stage_name(), "stage1");
  ASSERT_EQ(stage.get_line_size(), 2);
}
