// Simple main():
//  * creates and randmly fills two tables (small and big)
//  * serializes the small table into a file and destroys it.
//  * deserializes a table from the file.
//  * performs hash-join on the two tables, prints the result

// Build and run:
//   clang++  join_main.cpp -O2 -g && ./a.out
// (tested with clang 6)
// See also join_test.cpp


#include "join.h"

const std::size_t SizeSmall = 20;
const std::size_t SizeBig = 10000;

int main() {
  std::mt19937 mt { 0 };
  typedef std::tuple<int, double, std::string, nns::CString> Row1;
  typedef std::tuple<std::string, int, nns::CString, double> Row2;

  // "big"
  nns::Table<Row2> tbl_b;
  tbl_b.random(SizeBig, mt);
  //tbl_b.print(std::cout);

  {
    // "small"
    nns::Table<Row1> tbl_s;
    tbl_s.random(SizeSmall, mt);
    // tbl_s.print(std::cout);
    // serialize the small table
    std::ofstream ofs("nns_small", std::ios::binary);
    tbl_s.serialize(ofs);
    tbl_s.cleanup();
  }
  nns::Table<Row1> tbl_from_file;
  std::ifstream fs("nns_small", std::ios::binary);
  tbl_from_file.deserialize(fs);
  //tbl_from_file.print(std::cout);

  // join column indexes: 2 for small, 0 for big
  typedef nns::Joiner<2, Row1, 0, Row2>  Joiner;
  auto tbl = Joiner::table_join(tbl_from_file, tbl_b);
  std::cout <<  "--- table join 2 & 0 ----\n";
  tbl.print(std::cout);
  // cleanup
  tbl_b.cleanup();
  tbl_from_file.cleanup();
}
