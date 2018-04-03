#include "gtest/gtest.h"

#include "linear/any.h"
#include <sstream>

TEST(AnyTest, simple) {
  {
    linear::type::any a1, a2((linear::type::nil()));
    EXPECT_TRUE(a1.is_nil());
    EXPECT_EQ(linear::type::any::NIL, a1.type);
    EXPECT_TRUE(a2.is_nil());
    EXPECT_EQ(linear::type::any::NIL, a2.type);
  }
  {
    linear::type::any a1(false), a2(true);
    EXPECT_EQ(linear::type::any::BOOLEAN, a1.type);
    EXPECT_EQ(false, a1.as<bool>());
    EXPECT_EQ(linear::type::any::BOOLEAN, a2.type);
    EXPECT_EQ(true, a2.as<bool>());
  }
  {
    linear::type::any a1(-1), a2(0), a3(1);
    EXPECT_EQ(linear::type::any::NEGATIVE_INTEGER, a1.type);
    EXPECT_EQ(-1, a1.as<int>());
    EXPECT_EQ(linear::type::any::POSITIVE_INTEGER, a2.type);
    EXPECT_EQ(0, a2.as<int>());
    EXPECT_EQ(linear::type::any::POSITIVE_INTEGER, a3.type);
    EXPECT_EQ(1, a3.as<int>());
  }
#if MSGPACK_VERSION_MAJOR == 2 && MSGPACK_VERSION_MINOR >= 1 || MSGPACK_VERSION_MAJOR > 2
  {
    double d = 3.14;
    float f = 3.14f;
    linear::type::any a1(d);
    EXPECT_EQ(linear::type::any::FLOAT64, a1.type);
    EXPECT_EQ(d, a1.as<double>());
    linear::type::any a2(f);
    EXPECT_EQ(linear::type::any::FLOAT32, a2.type);
    EXPECT_EQ(f, a2.as<float>());
  }
#else
  {
    float f = 3.14;
    linear::type::any a1(f);
    EXPECT_EQ(linear::type::any::FLOAT, a1.type);
    EXPECT_EQ(f, a1.as<float>());
  }
#endif
  {
    std::string s = "test";
    linear::type::any a1(s);
    EXPECT_EQ(linear::type::any::STR, a1.type);
    EXPECT_EQ(s, a1.as<std::string>());
  }
  {
    linear::type::binary b("\x00\x01\x02", 3);
    linear::type::any a1(b);
    EXPECT_EQ(linear::type::any::BIN, a1.type);
    EXPECT_EQ(b, a1.as<linear::type::binary>());
  }
  {
    std::vector<int> v;
    v.push_back(-1);
    v.push_back(0);
    v.push_back(1);
    linear::type::any a1(v);
    EXPECT_EQ(linear::type::any::ARRAY, a1.type);
    EXPECT_EQ(v, a1.as<std::vector<int> >());
  }
  {
    std::map<std::string, int> m;
    m.insert(std::make_pair("k1", -1));
    m.insert(std::make_pair("k2", 0));
    m.insert(std::make_pair("k3", 1));
    linear::type::any a1(m);
    EXPECT_EQ(linear::type::any::MAP, a1.type);
    EXPECT_EQ(m, (a1.as<std::map<std::string, int> >()));
  }
}

TEST(AnyTest, anyVec) {
  {
    int i = 0;
    float f = 3.14;
    std::string s = "string";
    linear::type::binary b("\x00\x01\x02", 3);
    std::vector<int> iv;
    iv.push_back(-1);
    iv.push_back(0);
    iv.push_back(1);
    std::map<std::string, int> im;
    im.insert(std::make_pair("k1", -1));
    im.insert(std::make_pair("k2", 0));
    im.insert(std::make_pair("k3", 1));

    std::vector<linear::type::any> v;
    v.push_back(i);
    v.push_back(f);
    v.push_back(s);
    v.push_back(b);
    v.push_back(iv);
    v.push_back(im);
    linear::type::any a1(v);
    EXPECT_EQ(linear::type::any::ARRAY, a1.type);

    std::vector<linear::type::any> res;
    res = a1.as<std::vector<linear::type::any> >();
    EXPECT_EQ(linear::type::any::POSITIVE_INTEGER, res[0].type);
    EXPECT_EQ(i, (res[0].as<int>()));
#if MSGPACK_VERSION_MAJOR == 2 && MSGPACK_VERSION_MINOR >= 1 || MSGPACK_VERSION_MAJOR > 2
    EXPECT_EQ(linear::type::any::FLOAT32, res[1].type);
#else
    EXPECT_EQ(linear::type::any::FLOAT, res[1].type);
#endif
    EXPECT_EQ(f, (res[1].as<float>()));
    EXPECT_EQ(linear::type::any::STR, res[2].type);
    EXPECT_EQ(s, (res[2].as<std::string>()));
    EXPECT_EQ(linear::type::any::BIN, res[3].type);
    EXPECT_EQ(b, (res[3].as<linear::type::binary>()));
    EXPECT_EQ(linear::type::any::ARRAY, res[4].type);
    EXPECT_EQ(iv, (res[4].as<std::vector<int> >()));
    EXPECT_EQ(linear::type::any::MAP, res[5].type);
    EXPECT_EQ(im, (res[5].as<std::map<std::string, int> >()));
  }
}

TEST(AnyTest, anyMap) {
  {
    int i = 0;
    float f = 3.14;
    std::string s = "string";
    linear::type::binary b("\x00\x01\x02", 3);
    std::vector<int> iv;
    iv.push_back(-1);
    iv.push_back(0);
    iv.push_back(1);
    std::map<std::string, int> im;
    im.insert(std::make_pair("k1", -1));
    im.insert(std::make_pair("k2", 0));
    im.insert(std::make_pair("k3", 1));

    std::map<std::string, linear::type::any> m;
    m.insert(std::make_pair("k1", i));
    m.insert(std::make_pair("k2", f));
    m.insert(std::make_pair("k3", s));
    m.insert(std::make_pair("k4", b));
    m.insert(std::make_pair("k5", iv));
    m.insert(std::make_pair("k6", im));
    linear::type::any a1(m);
    EXPECT_EQ(linear::type::any::MAP, a1.type);

    std::map<std::string, linear::type::any> res;
    res = a1.as<std::map<std::string, linear::type::any> >();
    EXPECT_EQ(linear::type::any::POSITIVE_INTEGER, res["k1"].type);
    EXPECT_EQ(i, (res["k1"].as<int>()));
#if MSGPACK_VERSION_MAJOR == 2 && MSGPACK_VERSION_MINOR >= 1 || MSGPACK_VERSION_MAJOR > 2
    EXPECT_EQ(linear::type::any::FLOAT32, res["k2"].type);
#else
    EXPECT_EQ(linear::type::any::FLOAT, res["k2"].type);
#endif
    EXPECT_EQ(f, (res["k2"].as<float>()));
    EXPECT_EQ(linear::type::any::STR, res["k3"].type);
    EXPECT_EQ(s, (res["k3"].as<std::string>()));
    EXPECT_EQ(linear::type::any::BIN, res["k4"].type);
    EXPECT_EQ(b, (res["k4"].as<linear::type::binary>()));
    EXPECT_EQ(linear::type::any::ARRAY, res["k5"].type);
    EXPECT_EQ(iv, (res["k5"].as<std::vector<int> >()));
    EXPECT_EQ(linear::type::any::MAP, res["k6"].type);
    EXPECT_EQ(im, (res["k6"].as<std::map<std::string, int> >()));
  }
}

TEST(AnyTest, anyInAny) {
  {
    int i = 0;
    float f = 3.14;
    std::string s = "string";
    linear::type::binary b("\x00\x01\x02", 3);
    std::vector<linear::type::any> iv;
    iv.push_back(i);
    iv.push_back(f);
    iv.push_back(s);
    std::map<std::string, linear::type::any> im;
    im.insert(std::make_pair("k1", i));
    im.insert(std::make_pair("k2", f));
    im.insert(std::make_pair("k3", s));
    im.insert(std::make_pair("k4", iv));

    std::map<std::string, linear::type::any> m;
    m.insert(std::make_pair("k1", i));
    m.insert(std::make_pair("k2", f));
    m.insert(std::make_pair("k3", s));
    m.insert(std::make_pair("k4", b));
    m.insert(std::make_pair("k5", iv));
    m.insert(std::make_pair("k6", im));
    linear::type::any a1(m);
    EXPECT_EQ(linear::type::any::MAP, a1.type);

    std::map<std::string, linear::type::any> res;
    res = a1.as<std::map<std::string, linear::type::any> >();
    EXPECT_EQ(linear::type::any::POSITIVE_INTEGER, res["k1"].type);
    EXPECT_EQ(i, (res["k1"].as<int>()));
#if MSGPACK_VERSION_MAJOR == 2 && MSGPACK_VERSION_MINOR >= 1 || MSGPACK_VERSION_MAJOR > 2
    EXPECT_EQ(linear::type::any::FLOAT32, res["k2"].type);
#else
    EXPECT_EQ(linear::type::any::FLOAT, res["k2"].type);
#endif
    EXPECT_EQ(f, (res["k2"].as<float>()));
    EXPECT_EQ(linear::type::any::STR, res["k3"].type);
    EXPECT_EQ(s, (res["k3"].as<std::string>()));
    EXPECT_EQ(linear::type::any::BIN, res["k4"].type);
    EXPECT_EQ(b, (res["k4"].as<linear::type::binary>()));
    EXPECT_EQ(linear::type::any::ARRAY, res["k5"].type);
    std::vector<linear::type::any> resv = res["k5"].as<std::vector<linear::type::any> >();
    EXPECT_EQ(iv, resv);
    EXPECT_EQ(linear::type::any::MAP, res["k6"].type);
    std::map<std::string, linear::type::any> resm = res["k6"].as<std::map<std::string, linear::type::any> >();
    EXPECT_EQ(im, resm);
  }
}

TEST(AnyTest, operators) {
  {
    int i = 0;
    float f = 3.14;
    std::string s = "string";
    linear::type::binary b("\x00\x01\x02", 3);
    std::vector<linear::type::any> iv;
    iv.push_back(i);
    iv.push_back(f);
    iv.push_back(s);
    std::map<std::string, linear::type::any> im;
    im.insert(std::make_pair("k1", i));
    im.insert(std::make_pair("k2", f));
    im.insert(std::make_pair("k3", s));
    im.insert(std::make_pair("k4", iv));

    std::map<std::string, linear::type::any> m;
    m.insert(std::make_pair("k1", i));
    m.insert(std::make_pair("k2", f));
    m.insert(std::make_pair("k3", s));
    m.insert(std::make_pair("k4", b));
    m.insert(std::make_pair("k5", iv));
    m.insert(std::make_pair("k6", im));
    linear::type::any a1(m), a2(m), a3(a1), a4;

    EXPECT_TRUE(a1 == a2);
    EXPECT_TRUE(a1 == a3);
    EXPECT_TRUE(a1 != a4);
    a4 = im;
    EXPECT_FALSE(a1 == a4);
    a4 = a1;
    EXPECT_TRUE(a1 == a4);
  }
}

TEST(AnyTest, msgpack_object) {
  {
    msgpack::type::nil_t n;
    msgpack::object no(n);
    linear::type::any a1(n), a2(no);
    EXPECT_TRUE(a1.is_nil());
    EXPECT_EQ(linear::type::any::NIL, a1.type);
    EXPECT_TRUE(a2.is_nil());
    EXPECT_EQ(linear::type::any::NIL, a2.type);
  }
  {
    msgpack::object o1(false), o2(true);
    linear::type::any a1(o1), a2(o2);
    EXPECT_EQ(linear::type::any::BOOLEAN, a1.type);
    EXPECT_EQ(false, a1.as<bool>());
    EXPECT_EQ(linear::type::any::BOOLEAN, a2.type);
    EXPECT_EQ(true, a2.as<bool>());
  }
  {
    msgpack::object o1(-1), o2(0), o3(1);
    linear::type::any a1(o1), a2(o2), a3(o3);
    EXPECT_EQ(linear::type::any::NEGATIVE_INTEGER, a1.type);
    EXPECT_EQ(-1, a1.as<int>());
    EXPECT_EQ(linear::type::any::POSITIVE_INTEGER, a2.type);
    EXPECT_EQ(0, a2.as<int>());
    EXPECT_EQ(linear::type::any::POSITIVE_INTEGER, a3.type);
    EXPECT_EQ(1, a3.as<int>());
  }
  {
    float f = 3.14;
    msgpack::object o1(f);
    linear::type::any a1(o1);
#if MSGPACK_VERSION_MAJOR == 2 && MSGPACK_VERSION_MINOR >= 1 || MSGPACK_VERSION_MAJOR > 2
    EXPECT_EQ(linear::type::any::FLOAT32, a1.type);
#else
    EXPECT_EQ(linear::type::any::FLOAT, a1.type);
#endif
    EXPECT_EQ(f, a1.as<float>());
  }
  {
    std::string s = "test";
    msgpack::zone z;
    msgpack::object o1(s, z);
    linear::type::any a1(s);
    EXPECT_EQ(linear::type::any::STR, a1.type);
    EXPECT_EQ(s, a1.as<std::string>());
  }
  {
    linear::type::binary b("\x00\x01\x02", 3);
    msgpack::zone z;
    msgpack::object o1(b, z);
    linear::type::any a1(o1);
    EXPECT_EQ(linear::type::any::BIN, a1.type);
    EXPECT_EQ(b, a1.as<linear::type::binary>());
  }
  {
    std::vector<int> v;
    v.push_back(-1);
    v.push_back(0);
    v.push_back(1);
    msgpack::zone z;
    msgpack::object o1(v, z);
    linear::type::any a1(o1);
    EXPECT_EQ(linear::type::any::ARRAY, a1.type);
    EXPECT_EQ(v, a1.as<std::vector<int> >());
  }
  {
    std::map<std::string, int> m;
    m.insert(std::make_pair("k1", -1));
    m.insert(std::make_pair("k2", 0));
    m.insert(std::make_pair("k3", 1));
    msgpack::zone z;
    msgpack::object o1(m, z);
    linear::type::any a1(o1);
    EXPECT_EQ(linear::type::any::MAP, a1.type);
    EXPECT_EQ(m, (a1.as<std::map<std::string, int> >()));
  }
}

TEST(AnyTest, stringify) {
  {
    linear::type::nil n;
    int i = 0;
    float f = 3.14;
    std::string s = "Abｶﾅ漢字,～－<>&$①Ⅰ㍉覬鎹";
    linear::type::binary b("\x00\x01\x02\",", 5);
    std::vector<linear::type::any> iv;
    iv.push_back(n);
    iv.push_back(i);
    iv.push_back(f);
    iv.push_back(s);
    std::map<std::string, linear::type::any> im;
    im.insert(std::make_pair("k0", n));
    im.insert(std::make_pair("k1", i));
    im.insert(std::make_pair("k2", f));
    im.insert(std::make_pair("k3", s));
    im.insert(std::make_pair("k4", iv));

    std::map<std::string, linear::type::any> m;
    m.insert(std::make_pair("k0", n));
    m.insert(std::make_pair("k1", i));
    m.insert(std::make_pair("k2", f));
    m.insert(std::make_pair("k3", s));
    m.insert(std::make_pair("k4", b));
    m.insert(std::make_pair("k5", iv));
    m.insert(std::make_pair("k6", im));
    linear::type::any a1(m);

    std::string expect = "{\"k0\":null, \"k1\":0, \"k2\":3.14, \"k3\":\"Abｶﾅ漢字,～－<>&$①Ⅰ㍉覬鎹\", \"k4\":[BIN], \"k5\":[null, 0, 3.14, \"Abｶﾅ漢字,～－<>&$①Ⅰ㍉覬鎹\"], \"k6\":{\"k0\":null, \"k1\":0, \"k2\":3.14, \"k3\":\"Abｶﾅ漢字,～－<>&$①Ⅰ㍉覬鎹\", \"k4\":[null, 0, 3.14, \"Abｶﾅ漢字,～－<>&$①Ⅰ㍉覬鎹\"]}}";

    std::string expect_cut = "{\"k0\":null, \"k1\":0, \"k2\":3.14, \"k3\":\"Abｶﾅ漢字,～－<>&$①Ⅰ㍉覬鎹\", \"k4\":[BIN], \"k5\":[null, 0, 3.14, \"Ab\xEF\xBD\xB6\xEF\xBE\x85\xE6\xBC\xA2\xE5\xAD\x97...(truncated)";

    std::string raw = a1.stringify();
    std::cout << raw << std::endl;
    EXPECT_EQ(expect, raw);

    std::string raw_cut = a1.stringify(128);
    std::cout << raw_cut << std::endl;
    EXPECT_EQ(expect_cut, raw_cut);
  }
}

TEST(AnyTest, ext) {
  {
    msgpack::type::ext ext(10, "abcdef", 5);
    linear::type::any a(ext);
    std::string expect = "[EXT]";
    EXPECT_EQ(expect, a.stringify());
    std::cout << a.stringify() << std::endl;
    std::map<std::string, linear::type::any> m;
    m.insert(std::make_pair("k0", a));
    linear::type::any resa = m["k0"];
    EXPECT_EQ(expect, resa.stringify());
    msgpack::type::ext res = resa.as<msgpack::type::ext>();
    EXPECT_EQ(ext, res);
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
