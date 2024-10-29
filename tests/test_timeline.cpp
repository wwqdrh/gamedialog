#include <gtest/gtest.h>

#include "timeline.h"

using namespace gamedialog;

TEST(TimelineTest, testparse) {
  Timeline parser(R"(
[stage1]
(John)
Hello there!
-Yes: next_scene
-No: end_scene

[next_scene]
(Mary)
answer yes!!

[end_scene]
answer no!!
)");

  auto stages = parser.all_stages();
  ASSERT_EQ(3, stages.size());

  // 执行一次调用
  auto word = parser.next();
  ASSERT_TRUE(word != nullptr);
  ASSERT_EQ("John", word->get_name());
  ASSERT_EQ("Hello there!", word->get_text());
  ASSERT_EQ(2, word->get_responses().size());

  // 手动跳转
  parser.goto_stage("next_scene");
  auto word2 = parser.next();
  ASSERT_EQ("Mary", word2->get_name());
  ASSERT_EQ("answer yes!!", word2->get_text());

  // 回到开头
  parser.goto_begin();
  ASSERT_EQ(parser.current_stage(), "stage1");
}


TEST(TimelineTest, TestControlFlow) {
  // 各种控制分支
  // :start 回到开头
  // :end 表示退出
  // :skip:1 表示跳过n个stage
  // :goto:name 跳转到指定的stage
  Timeline parser(R"(
[stage1]
(John)
Hello there!
:skip:2

[next_scene]
(Mary)
answer yes!!
:end

[end_scene]
(John)
answer no!!
:start

[goto_scene]
(John)
answer no!!
:goto:noexist
)");

  auto stages = parser.all_stages();
  ASSERT_EQ(4, stages.size());
  parser.next();
  ASSERT_EQ(parser.current_stage(), "stage1");
  parser.next(); // skip:2 + next
  ASSERT_EQ(parser.current_stage(), "end_scene");
  parser.next(); // :start + next
  ASSERT_EQ(parser.current_stage(), "stage1");

  // 测试end
  parser.goto_stage("next_scene");
  ASSERT_EQ(parser.current_stage(), "next_scene");
  ASSERT_TRUE(parser.has_next());
  parser.next();
  ASSERT_FALSE(parser.has_next()); // 当前是:end标签，那么就没有下一个元素了

  // 测试goto
  parser.goto_stage("goto_scene");
  ASSERT_EQ(parser.current_stage(), "goto_scene");
  ASSERT_TRUE(parser.has_next());
  parser.next();
  ASSERT_FALSE(parser.has_next()); // 当前是:end标签，那么就没有下一个元素了
}
