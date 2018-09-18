#include "gtest/gtest.h"
#include "join.h"


TEST(RowUtil, print) {
  typedef std::tuple<int, double, std::string, const char*> Row;
  Row row = std::make_tuple(42, 3.14, "asdf", "qqqmmm");
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
  nns::RowUtil::print(row_out, std::cout);
  nns::RowUtil::cleanup(row_out);
}

TEST(RowUtil, serialize1) {
  typedef std::tuple<int, double> Row;
  Row row = std::make_tuple(42, 3.14);
  testRowUtil(row);
}

TEST(RowUtil, serialize2) {
  typedef std::tuple<int, double, std::string> Row;
  Row row = std::make_tuple(42, 3.14, "asdf");
  testRowUtil(row);
}

TEST(RowUtil, serialize3) {
  typedef std::tuple<nns::CString> Row;
  Row row = std::make_tuple(nns::CString("asas"));
  testRowUtil(row);
}

TEST(RowUtil, serialize4) {
  typedef std::tuple<const char*, int> Row;
  const char *str = "asas";
  Row row = std::make_tuple(str, 3);
  testRowUtil(row);
}

TEST(RowUtil, serialize5) {
  typedef std::tuple<long, long double, std::string, int, const char*> Row;
  Row row = std::make_tuple(42, 3.14, std::string("asdf"), 45, "asas");
  testRowUtil(row);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}
