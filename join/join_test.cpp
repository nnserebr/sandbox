#include "gtest/gtest.h"
#include "join.h"


/*
void test_sorter(std::vector<std::string> v){
  bigstream::sorter<decltype(Key)> my_sorter(Key, 20);
  for (auto &s : v) {
    //std::cout << "test " << s << "\n";
    bigstream::Row row = S2R(s);
    my_sorter.insert_row(row);
  }
  my_sorter.end_insertion();
  std::vector<bigstream::Row> result;
  bigstream::Row row = my_sorter.next_sorted_row();
  while (std::get<1>(row)) {
    result.push_back(row);
    //std::cout << "out " << my_sorter.R2S(row) << std::endl;
    row = my_sorter.next_sorted_row();
  }

  EXPECT_TRUE(std::is_sorted(result.begin(), result.end(),
        [](bigstream::Row r1, bigstream::Row r2){
        return (Key(r1) < Key(r2) );
        }));

  for (auto r : result)
    delete [] (char*)std::get<0>(r);
}


TEST(Sorter, T1) {
  test_sorter({"ABCD", "A", "EFGHIJK", "BCCCCCCCCCC", "SSSSSSSSSSSSS", "KK" });
}
*/

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
  typedef std::tuple<long, long double, std::string, int, nns::CString> Row;
  Row row = std::make_tuple(42, 3.14, std::string("asdf"), 45, nns::CString("asas"));
  testRowUtil(row);
}
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}
