#include <cstring>

#include "gtest/gtest.h"

#include "linear/optional.h"

TEST(OptionalTest, ConstructorAndConversionOperatorWorks) {
  linear::type::optional<int> o1;
  EXPECT_FALSE(bool(o1));
  linear::type::optional<int> o2( (linear::type::nil()) );
  EXPECT_FALSE(bool(o2));
  linear::type::optional<int> o3(1);
  EXPECT_TRUE(bool(o3));
}

TEST(OptionalTest, CopyConstructorWorks) {
  linear::type::optional<int> o1(10);
  EXPECT_TRUE(bool(o1));
  linear::type::optional<int> o2(o1);
  EXPECT_TRUE(bool(o2));
  EXPECT_EQ(10, *o2);

  linear::type::optional<int> o3;
  EXPECT_FALSE(bool(o3));
  linear::type::optional<int> o4(o3);
  EXPECT_FALSE(bool(o4));
}

TEST(OptionalTest, AssignmentAndIndirectionOperatorWorks) {
  linear::type::optional<int> o1;
  linear::type::optional<int> o2;

  o1 = 1;
  EXPECT_TRUE(bool(o1));
  EXPECT_EQ(1, *o1);

  o1 = linear::type::nil();
  EXPECT_FALSE(bool(o1));

  o1 = linear::type::nil();
  o2 = linear::type::nil();
  o1 = o2;
  EXPECT_FALSE(bool(o1));

  o1 = 1;
  o2 = linear::type::nil();
  o1 = o2;
  EXPECT_FALSE(bool(o1));

  o1 = 1;
  o2 = 2;
  o1 = o2;
  EXPECT_TRUE(bool(o1));
  EXPECT_EQ(2, *o1);
}

TEST(OptionalTest, StructureDeferenceOperatorWorks) {
  std::string str("string");
  linear::type::optional<std::string> o1(str);
  EXPECT_EQ(str.size(), o1->size());
  EXPECT_EQ(str.append("string"), o1->append("string"));
}

TEST(OptionalTest, ValueMethodWorks) {
  linear::type::optional<int> o1;
  EXPECT_THROW({ o1.value(); }, std::logic_error);

  linear::type::optional<int> o2(1);
  EXPECT_NO_THROW(o2.value());
  EXPECT_EQ(1, o2.value());
}

TEST(OptionalTest, ValueOrMethodWorks) {
  linear::type::optional<int> o1;
  EXPECT_EQ(-1, o1.value_or(-1));

  linear::type::optional<int> o2(1);
  EXPECT_EQ(1, o2.value_or(-1));
}

TEST(OptionalTest, EqualToOperatorWorks) {
  linear::type::optional<int> o1;
  EXPECT_TRUE(o1 == linear::type::nil());
  EXPECT_FALSE(linear::type::nil() == o1);
  EXPECT_FALSE(o1 == 1);
  // EXPECT_FALSE(o1 == 2);
  EXPECT_FALSE(1 == o1);
  // EXPECT_FALSE(2 == o1);

  linear::type::optional<int> o2(1);
  EXPECT_FALSE(o2 == linear::type::nil());
  EXPECT_FALSE(linear::type::nil() == o2);
  EXPECT_TRUE(o2 == 1);
  EXPECT_FALSE(o2 == 2);
  EXPECT_TRUE(1 == o2);
  EXPECT_FALSE(2 == o2);

  EXPECT_FALSE(o1 == o2);
  EXPECT_FALSE(o2 == o1);
  linear::type::optional<int> o3;
  EXPECT_TRUE(o1 == o3);
  linear::type::optional<int> o4(1);
  EXPECT_TRUE(o2 == o4);
  linear::type::optional<int> o5(2);
  EXPECT_FALSE(o2 == o5);
}

TEST(OptionalTest, LessThanOperatorWorks) {
  linear::type::optional<int> o1;
  EXPECT_TRUE(o1 < linear::type::nil());
  EXPECT_FALSE(linear::type::nil() < o1);
  EXPECT_TRUE(o1 < 1);
  // EXPECT_TRUE(o1 < 2);

  linear::type::optional<int> o2(1);
  EXPECT_FALSE(o2 < linear::type::nil());
  EXPECT_TRUE(linear::type::nil() < o2);
  EXPECT_FALSE(o2 < 1);
  EXPECT_TRUE(o2 < 2);

  EXPECT_TRUE(o1 < o2);
  EXPECT_FALSE(o2 < o1);
  linear::type::optional<int> o3;
  EXPECT_FALSE(o1 < o3);
  linear::type::optional<int> o4(1);
  EXPECT_FALSE(o2 < o4);
  linear::type::optional<int> o5(2);
  EXPECT_TRUE(o2 < o5);
}

TEST(OptionalTest, SerializeWorks) {
  linear::type::optional<int> o1;
  msgpack::sbuffer sbuf1;
  msgpack::pack(sbuf1, o1);

  msgpack::sbuffer sbuf2;
  msgpack::pack(sbuf2, linear::type::nil());

  EXPECT_EQ(sbuf1.size(), sbuf2.size());
  EXPECT_EQ(0, memcmp(sbuf1.data(), sbuf2.data(), sbuf1.size()));

  linear::type::optional<int> o3(1);
  msgpack::sbuffer sbuf3;
  msgpack::pack(sbuf3, o3);

  msgpack::sbuffer sbuf4;
  msgpack::pack(sbuf4, 1);

  EXPECT_EQ(sbuf1.size(), sbuf2.size());
  EXPECT_EQ(0, memcmp(sbuf1.data(), sbuf2.data(), sbuf1.size()));
}

TEST(OptionalTest, DeserializeWorks) {
  msgpack::sbuffer sbuf1;
  msgpack::pack(sbuf1, linear::type::nil());

  msgpack::unpacked result1;
  msgpack::unpack(&result1, sbuf1.data(), sbuf1.size());
  msgpack::object obj1 = result1.get();

  linear::type::optional<int> o1;
  obj1.convert(&o1);
  EXPECT_FALSE(bool(o1));

  msgpack::sbuffer sbuf2;
  msgpack::pack(sbuf2, 1);

  msgpack::unpacked result2;
  msgpack::unpack(&result2, sbuf2.data(), sbuf2.size());
  msgpack::object obj2 = result2.get();

  linear::type::optional<int> o2;
  obj2.convert(&o2);
  EXPECT_TRUE(bool(o2));
  EXPECT_EQ(1, *o2);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
