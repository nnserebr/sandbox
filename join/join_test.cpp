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
  typedef std::tuple<int, double, std::string, nns::CString> Row;
  Row row;
  nns::RowUtil::random(row);
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
  typedef std::tuple<int, double, std::string, nns::CString> Row;
  nns::Table<Row> tbl;
  for (int i = 0; i < 10; i++) {
    Row row;
    nns::RowUtil::random(row);
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
  tbl1.print(std::cout);
  tbl2.print(std::cout);
  auto tbl = nns::table_product(tbl1, tbl2);
  tbl.print(std::cout);
}

int main(int argc, char **argv) {
    srand(1);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
