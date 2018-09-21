#include <vector>
#include <tuple>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <random>
#include <iterator>
#include <algorithm>
#include <unordered_map>
#include <assert.h>

// ============== CString ================

namespace nns {
  // Wrapper for 'const char*' to behave like other types with operator== and hash.
  // Does not own its 'const char*' object.
  class CString {
    const char *_s;
   public:
    CString(const char *s=nullptr):_s(s){}
    bool operator==(const CString &t) const { return !std::strcmp(_s, t._s);}
    const char *get() const {return _s;}
    void set(const char *s){ _s = s; }
  };

  std::ostream & operator << (std::ostream &os, const CString &str) {
    return os << str.get();
  }
}

// Hash for CString.
namespace std {
  template <>
  struct hash<nns::CString>
  {
    std::size_t operator()(const nns::CString &s) const
    {
      const char *p = s.get();
      std::size_t res = 0;
      while(*p) {
        res = *(p++) + res*31;
      }
      return res;
    }
  };
}

// ============== Row ====================

namespace nns {
  // Row type is simply std::tuple<type1, type2, ...>.
  // RowUtil provides functions to handle a Row.
  // (I've tried to wrap Row into a class with member functions,
  //  but the level of template magic was beyond my current skill :)

  class RowUtil {
    // Serialization of numeric types.
    template<typename T>
    static void serializeT(const T &item, std::ostream &os) {
      os.write((char *) &item, sizeof(item));
    }
    // Serialization of string.
    static void serializeT(const std::string &s, std::ostream &os) {
      std::size_t size = s.size();
      os.write((char *) &size, sizeof(size));
      os.write(s.data(), size);
    }
    // Serialization of CString.
    static void serializeT(const CString &s, std::ostream &os) {
      std::size_t size = std::strlen(s.get());
      os.write((char *) &size, sizeof(size));
      os.write(s.get(), size);
    }

    // Deserialization of numeric types.
    template<typename T>
    static void deserializeT(T &item, std::ifstream &fs) {
      fs.read((char*)&item, sizeof(item));
    }
    // Deserialization of string.
    static void deserializeT(std::string &s, std::ifstream &fs) {
      std::size_t size;
      fs.read((char*) &size, sizeof(size));
      if (!fs.good()) return;
      std::vector<char> v(size);
      fs.read(v.data(), size);
      s.append(v.begin(), v.end());
    }
    // Deserialization of CString. Allocates memory but doesn't frees it.
    static void deserializeT(CString &str, std::ifstream &fs) {
      std::size_t size;
      fs.read((char*) &size, sizeof(size));
      if (!fs.good()) return;
      char *buf = new char[size+1];
      fs.read(buf, size);
      buf[size] = 0;
      str.set(buf);
    }

    template<typename T>
    static void cleanupT(const T &item){}

    static void cleanupT(const CString &str) {
      delete [] str.get();
    }

    template<typename T>
    static void randomT(T &item, std::mt19937 &mt) {
      item = -50 + mt() % 100;
    }
    static void randomT(double &item, std::mt19937 &mt) {
      item = (mt() % 100) * 0.1 - 5.;
    }
    // From somewhere https://www.reddit.com/r/cpp_questions/comments/22p1e6/random_string_generator_in_c/
    static void randomT(std::string &s, std::mt19937 &mt) {
      std::size_t len = 3;
      std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      std::uniform_int_distribution<int> dist(0, characters.length() - 1);
      for (size_t i = 0; i < len; ++i) {
        s += characters[dist(mt)];
      }
    }
    static void randomT(CString &str, std::mt19937 &mt) {
      std::string s;
      randomT(s, mt);
      char *buf = new char[s.size()+1];
      std::strcpy(buf, s.c_str());
      str.set(buf);
    }

    // This code was inspired by
    // https://en.cppreference.com/w/cpp/utility/tuple/tuple_cat
    // Helper functions to iterate over tuple.
    template<typename Row, std::size_t N>
    struct TupleVisitor {
      static void print(const Row &row, std::ostream &os) {
        TupleVisitor<Row, N-1>::print(row, os);
        os << ", " << std::get<N-1>(row);
      }
      static void serialize(const Row &row, std::ofstream &fs) {
        TupleVisitor<Row, N-1>::serialize(row, fs);
        serializeT(std::get<N-1>(row), fs);
      }
      static void deserialize(Row &row, std::ifstream &fs) {
        TupleVisitor<Row, N-1>::deserialize(row, fs);
        deserializeT(std::get<N-1>(row), fs);
      }
      static void cleanup(const Row &row) {
        TupleVisitor<Row, N-1>::cleanup(row);
        cleanupT(std::get<N-1>(row));
      }
      static void random(Row &row, std::mt19937 &mt) {
        TupleVisitor<Row, N-1>::random(row, mt);
        randomT(std::get<N-1>(row), mt);
      }
    };

    template<typename Row>
    struct TupleVisitor<Row, 1> {
      static void print(const Row &row, std::ostream &os) {
        os << std::get<0>(row);
      }
      static void serialize(const Row &row, std::ofstream &fs) {
        serializeT(std::get<0>(row), fs);
      }
      static void deserialize(Row &row, std::ifstream &fs) {
        deserializeT(std::get<0>(row), fs);
      }
      static void cleanup(const Row &row){
        cleanupT(std::get<0>(row));
      }
      static void random(Row &row, std::mt19937 &mt) {
        randomT(std::get<0>(row), mt);
      }
    };

   public:

    // Prints tuple.
    template<typename Row>
    static void print(const Row &row, std::ostream &os) {
      os << "(";
      TupleVisitor<Row, std::tuple_size<Row>::value>::print(row, os);
      os << ")" << std::endl;
    }

    // Serializes tuple.
    template<typename Row>
    static void serialize(const Row &row, std::ofstream &fs) {
      TupleVisitor<Row, std::tuple_size<Row>::value>::serialize(row, fs);
    }

    // Deserializes tuple.
    template<typename Row>
    static void deserialize(Row &row, std::ifstream &fs) {
      TupleVisitor<Row, std::tuple_size<Row>::value>::deserialize(row, fs);
    }

    // Searches for CString type and frees allocated memory.
    // Row::~Row doesn't destroy CString objects stored in it
    // because they can be owned by the caller.
    // Call this if the Row owns it's CString objects e.g.
    // if Row is filled by deserialize() or random().
    template<typename Row>
    static void cleanup(const Row &row) {
      TupleVisitor<Row, std::tuple_size<Row>::value>::cleanup(row);
    }

    template<typename Row>
    static void random(Row &row, std::mt19937 &mt){
      TupleVisitor<Row, std::tuple_size<Row>::value>::random(row, mt);
    }
  };

// ============== Table ==================

  template<typename Row>
  class Table {
    std::vector<Row> _table;
   public:
    Table():_table({}){}
    void print (std::ostream &os) {
      os << "---- table begin -----\n";
      for(auto &row: _table)
        RowUtil::print(row, os);
      os << "---- table end -----\n";
    }
    const std::vector<Row> & get() const {
      return _table;
    }
    void add(const Row &row) {
      _table.push_back(row);
    }
    void serialize(std::ofstream &fs) {
      for(auto &row: _table)
        RowUtil::serialize(row, fs);
    }
    void deserialize(std::ifstream &fs) {
      assert(_table.empty());
      // No real error handling here, assuming the input is valid.
      while(true){
        Row row;
        RowUtil::deserialize(row, fs);
        if(!fs) return;
        add(row);
      }
    }
    void cleanup() {
      for(auto &row: _table)
        RowUtil::cleanup(row);
    }
    void clear() {
      _table.clear();
    }
    void random(std::size_t size, std::mt19937 &mt) {
      for (std::size_t i = 0; i < size; i++) {
        Row row;
        RowUtil::random(row, mt);
        add(row);
      }
    }
  };


// ============== Joiner =================

  // Function of no use here, just was training to write a function with auto return type
  template<typename Row1, typename Row2>
  auto table_product(const Table<Row1> &tbl1, const Table<Row2> &tbl2)
    -> Table<decltype(std::tuple_cat(Row1(), Row2()))>
  {
    Table<decltype(std::tuple_cat(Row1(), Row2()))> tbl;
    for (auto &row1 : tbl1.get()) {
      for (auto &row2 : tbl2.get()) {
        tbl.add(std::tuple_cat(row1, row2));
      }
    }
    return tbl;
  }

  // KeyIdx1, KeyIdx2 - numbers of columns to check the equality on
  // Row1, Row2 - types of rows of 2 tables
  // Hash and Cmp - optional hasher and comparator
  template<std::size_t KeyIdx1, typename Row1, std::size_t KeyIdx2, typename Row2,
                      typename Hash=std::hash<typename std::tuple_element<KeyIdx1, Row1>::type>,
                      typename Cmp=std::equal_to<typename std::tuple_element<KeyIdx1, Row1>::type>>
  struct Joiner {
    typedef typename std::tuple_element<KeyIdx1, Row1>::type KeyType1;

    typedef std::unordered_multimap<const KeyType1, const Row1 *, const Hash &, const Cmp &> MMap;

    // Returns multimap key : Row
    static MMap make_table_map(const Table<Row1> &tbl,
             const Hash &hasher=std::hash<KeyType1>(), const Cmp &cmp=std::equal_to<KeyType1>()) {
      MMap mmap(1, hasher, cmp);
      for(auto &row : tbl.get()) {
        mmap.insert(std::make_pair(std::get<KeyIdx1>(row), &row));
      }
      return mmap;
    }

    // Joins tbl1 and tbl2 into new table with rows combined by
    // checking equality on KeyIdx1 column of tbl1 and KeyIdx2 of column2
    typedef decltype(std::tuple_cat(Row1(), Row2())) RowCat;
    static Table<RowCat> table_join(const Table<Row1> &tbl1, const Table<Row2> &tbl2,
                const Hash &hasher=std::hash<KeyType1>(), const Cmp &cmp=std::equal_to<KeyType1>()) {
      MMap mmap = make_table_map(tbl1, hasher, cmp);
      Table<RowCat> tbl;
      for(auto &row2 : tbl2.get()) {
        auto range = mmap.equal_range(std::get<KeyIdx2>(row2));
        for_each (range.first, range.second,
            [&](typename MMap::value_type &pair){
                 tbl.add(std::tuple_cat(*(pair.second), row2)); });
      }
      return tbl;
    }
  };
}
