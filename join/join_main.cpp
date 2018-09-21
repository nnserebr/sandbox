// Simple main():
//   Creates and randmly fills two tables (small and big)
//   Serializes the small table into a file and destroys it.
//   Deserializes a table from the file.
//   Performs hash-join on the two tables, prints the result

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
  //std::ofstream b("b");
  //tbl_b.print(b);
  {
    // "small"
    nns::Table<Row1> tbl_s;
    tbl_s.random(SizeSmall, mt);
    //tbl_s.print(std::cout);
    //std::ofstream s("s");
    //tbl_s.print(s);
    std::ofstream ofs("nns_small", std::ios::binary);
    tbl_s.serialize(ofs);
    tbl_s.cleanup();
  }
  nns::Table<Row1> tbl_from_file;
  std::ifstream fs("nns_small", std::ios::binary);
  tbl_from_file.deserialize(fs);
  //tbl_from_file.print(std::cout);

  // case - insensitive comparator & hash (not the best)
  auto my_strcmp = [](const std::string &s1, const std::string &s2){
                          return !strcasecmp(s1.c_str(), s2.c_str()); };
  auto my_strhash = [](const std::string &s) {
                          std::size_t res = 0;
                          for (std::size_t i = 0; i < s.length(); i++)
                            res += (i + 1) * std::tolower(s[i]);
                          return res;   };

  { // Standart string cmp
    // Joins columns by indexes (2 for small, 0 for big).
    typedef nns::Joiner<2, Row1, 0, Row2>  Joiner;
    auto tbl = Joiner::table_join(tbl_from_file, tbl_b);
    std::cout <<  "Table join 2 & 0.\n";
    tbl.print(std::cout);
  }
  { // Case-insensitive string cmp
    // Joins columns by indexes (2 for small, 0 for big) using custom comparator.
    typedef nns::Joiner<2, Row1, 0, Row2, decltype(my_strhash), decltype(my_strcmp)>  Joiner;
    auto tbl = Joiner::table_join(tbl_from_file, tbl_b, my_strhash, my_strcmp);
    std::cout <<  "Table join 2 & 0 case-insensitive.\n";
    tbl.print(std::cout);
  }
  // cleanup
  tbl_b.cleanup();
  tbl_from_file.cleanup();
}
