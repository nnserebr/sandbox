#include "join.h"

const std::size_t SizeSmall = 20;
const std::size_t SizeBig = 10000;

int main() {
  std::mt19937 mt { 0 };
  typedef std::tuple<int, double, std::string, nns::CString> Row1;
  typedef std::tuple<std::string, int, nns::CString, double> Row2;

  // first table - "small"
  nns::Table<Row1> tbl_s;
  tbl_s.random(SizeSmall, mt);
  tbl_s.print(std::cout);

  // secon table - "big"
  nns::Table<Row2> tbl_b;
  tbl_b.random(SizeBig, mt);
  //tbl_b.print(std::cout);

  // serialization - deserialization of small table
  std::ofstream ofs("nns_small", std::ios::binary);
  tbl_s.serialize(ofs);
  ofs.close();
  tbl_s.cleanup();
  tbl_s.clear();
  std::ifstream fs("nns_small", std::ios::binary);
  tbl_s.deserialize(fs);
  tbl_s.print(std::cout);

  // join column indexes: 2 for small, 0 for big
  typedef nns::Joiner<2, Row1, 0, Row2>  Joiner;
  auto tbl = Joiner::table_join(tbl_s, tbl_b);
  std::cout <<  "--- table join 2 & 0 ----\n";
  tbl.print(std::cout);
  // cleanup
  tbl_b.cleanup();
  tbl_s.cleanup();
}
