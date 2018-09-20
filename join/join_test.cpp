#include "gtest/gtest.h"
#include "join.h"


TEST(RowUtil, print) {
  typedef std::tuple<int, double, std::string, nns::CString> Row;
  Row row = std::make_tuple(42, 3.14, "asdf", nns::CString("qqqmmm"));
  std::stringstream ss;
  nns::RowUtil::print(row, ss);
  EXPECT_EQ(ss.str(), "(42, 3.14, asdf, qqqmmm)\n");
}

template<typename Row>
void testRowUtil(Row &row) {
  nns::RowUtil::print(row, std::cout);
  {
    std::ofstream fs("f1", std::ios::binary);
    nns::RowUtil::serialize(row, fs);
  }
  Row row_out;
  std::ifstream fs("f1", std::ios::binary);
  nns::RowUtil::deserialize(row_out, fs);
  EXPECT_EQ(row, row_out);
  //if (!(row == row_out)) std::cout << "zzz\n";
  nns::RowUtil::print(row_out, std::cout);
  nns::RowUtil::cleanup(row_out);
}

TEST(RowUtil, file1) {
  typedef std::tuple<int, double> Row;
  Row row = std::make_tuple(42, 3.14);
  testRowUtil(row);
}

TEST(RowUtil, file2) {
  typedef std::tuple<int, double, std::string> Row;
  Row row = std::make_tuple(42, 3.14, "asdf");
  testRowUtil(row);
}

TEST(RowUtil, file3) {
  typedef std::tuple<int, double, nns::CString> Row;
  Row row = std::make_tuple(42, 3.14, nns::CString("asdf"));
  testRowUtil(row);
}
/*
TEST(RowUtil, file4) {
  typedef std::tuple<const char*, int> Row;
  const char *str = "asssssssssssssssssssssssssssssssssssssssas";
  Row row = std::make_tuple(str, 3);
  testRowUtil(row);
}
*/
TEST(RowUtil, file5) {
  typedef std::tuple<long, long double, std::string, int, nns::CString> Row;
  Row row = std::make_tuple(42, 3.14, std::string("asdf"), 45, nns::CString("asas"));
  testRowUtil(row);
}


TEST(RowUtil, rand) {
  std::mt19937 mt { 0 };
  //std::mt19937 mt { std::random_device {} () };
  typedef std::tuple<int, double, std::string, nns::CString> Row;
  Row row;
  nns::RowUtil::random(row, mt);
  nns::RowUtil::print(row, std::cout);
  nns::RowUtil::cleanup(row);
}


TEST(Table, add_print) {
  typedef std::tuple<long, long double, std::string, int, nns::CString> Row;
  nns::Table<Row> tbl;
  tbl.add(std::make_tuple(42, 3.14, std::string("asdf"), 45, nns::CString("asas")));
  tbl.add(std::make_tuple(42, 3.14, std::string("asdf"), 45, nns::CString("asas")));
  tbl.print(std::cout);
}

TEST(Table, file) {
  std::mt19937 mt { 0 };
  typedef std::tuple<int, double, std::string, nns::CString> Row;
  nns::Table<Row> tbl;
  for (int i = 0; i < 10; i++) {
    Row row;
    nns::RowUtil::random(row, mt);
    tbl.add(row);
  }
  tbl.print(std::cout);
  std::ofstream ofs("f2", std::ios::binary);
  tbl.serialize(ofs);
  ofs.close();
  tbl.cleanup();
  std::cout << "-----\n";
  std::ifstream ifs("f2", std::ios::binary);
  nns::Table<Row> tbl_out;
  tbl_out.deserialize(ifs);
  tbl_out.print(std::cout);
  tbl_out.cleanup();
}

TEST(Table, cat) {
  typedef std::tuple<int, std::string> Row1;
  typedef std::tuple<int, double, std::string> Row2;
  nns::Table<Row1> tbl1;
  nns::Table<Row2> tbl2;
  tbl1.add(std::make_tuple(2, "a"));
  tbl1.add(std::make_tuple(3, "b"));
  tbl2.add(std::make_tuple(2, 3.2, "a"));
  tbl2.add(std::make_tuple(4, 5.2, "r"));
  tbl2.add(std::make_tuple(7, 3.7, "c"));
  std::cout <<  "--- table 1 ----\n";
  tbl1.print(std::cout);
  std::cout <<  "--- table 2 ----\n";
  tbl2.print(std::cout);
  auto tbl = nns::table_product(tbl1, tbl2);
  std::cout <<  "--- table product ----\n";
  tbl.print(std::cout);
}


typedef std::tuple<int, double, std::string, nns::CString> Row;
template<std::size_t KeyIdx, typename T>
void testTableMap(T d1, T d2, T d3, const nns::Table<Row> &tbl, const Row &row) {
    typedef nns::Joiner<KeyIdx, Row, 0, Row>  Joiner;
    auto m = Joiner::make_table_map(tbl);
    EXPECT_EQ(m.size(), 6);
    {
      int c = m.count(d1);
      EXPECT_EQ(c, 1);
      auto range = m.equal_range(d1);
      for_each (range.first, range.second,
          [&](typename Joiner::MMap::value_type &pair){
                nns::RowUtil::print(*(pair.second), std::cout);
                EXPECT_EQ(*(pair.second), row); });
    }
    {
      int c = m.count(d2);
      EXPECT_EQ(c, 2);
      auto range = m.equal_range(d2);
      for_each (range.first, range.second,
          [&](typename Joiner::MMap::value_type &pair){
                nns::RowUtil::print(*(pair.second), std::cout); });
    }
    {
      int c = m.count(d3);
      EXPECT_EQ(c, 0);
    }
}

TEST(Table, map) {
  nns::Table<Row> tbl;
  tbl.add(std::make_tuple(2, 3.2, "a", "qqq"));
  tbl.add(std::make_tuple(4, 5.2, "r", "www"));
  tbl.add(std::make_tuple(7, 3.7, "c", "e"));
  tbl.add(std::make_tuple(2, 6.2, "t", "qqq"));
  tbl.add(std::make_tuple(8, 8.2, "r", "mmm"));
  tbl.add(std::make_tuple(9, 9.7, "c", "e"));
  std::cout <<  "--- table  ----\n";
  tbl.print(std::cout);
  std::cout <<  "--- int : 4, 2, 0 ----\n";
  testTableMap<0, int>(4, 2, 0, tbl, std::make_tuple(4, 5.2, "r", "www"));
  std::cout <<  "--- CString : 'www', 'e', 'abc' ----\n";
  testTableMap<3, nns::CString>("www", "e", "abc", tbl, std::make_tuple(4, 5.2, "r", "www"));
  std::cout <<  "--- string: 'a', 'c', 'z' ----\n";
  testTableMap<2, std::string>("a", "c", "z", tbl, std::make_tuple(2, 3.2, "a", "qqq"));

}

TEST(table, join) {
  typedef std::tuple<int, double, std::string, nns::CString> Row1;
  nns::Table<Row1> tbl1;
  tbl1.add(std::make_tuple(2, 3.2, "a", "qqq"));
  tbl1.add(std::make_tuple(4, 5.2, "r", "www"));
  tbl1.add(std::make_tuple(7, 3.7, "a", "e"));
  tbl1.add(std::make_tuple(2, 6.2, "t", "qqq"));
  std::cout <<  "--- table 1 ----\n";
  tbl1.print(std::cout);

  typedef std::tuple<std::string, int, nns::CString, double> Row2;
  nns::Table<Row2> tbl2;
  tbl2.add(std::make_tuple("a", 3, "vvv", 4.1));
  tbl2.add(std::make_tuple("a", 2, "bbb", 4.1));
  tbl2.add(std::make_tuple("b", 3, "qqq", 4.1));
  tbl2.add(std::make_tuple("c", 4, "www", 4.1));
  tbl2.add(std::make_tuple("r", 5, "aaa", 4.1));
  tbl2.add(std::make_tuple("r", 6, "bbb", 4.1));
  tbl2.add(std::make_tuple("x", 7, "ddd", 4.1));
  tbl2.add(std::make_tuple("z", 6, "mmm", 4.1));
  std::cout <<  "--- table 2 ----\n";
  tbl2.print(std::cout);
  {
    typedef nns::Joiner<2, Row1, 0, Row2>  Joiner;
    auto tbl = Joiner::table_join(tbl1, tbl2);
    std::cout <<  "--- table join 2 & 0 ----\n";
    tbl.print(std::cout);
  }
  {
    typedef nns::Joiner<3, Row1, 2, Row2>  Joiner;
    auto tbl = Joiner::table_join(tbl1, tbl2);
    std::cout <<  "--- table join 3 & 2 ----\n";
    tbl.print(std::cout);
  }
}

int main(int argc, char **argv) {
    srand(1);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
