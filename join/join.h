#include <vector>
#include <tuple>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

namespace nns {
  // wrapper for 'const char*' to behave like all normal types with ==
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

  class RowUtil {
    // serialization of numeric types
    template<typename T> 
    static void serializeT(const T &item, std::ostream &os) {
      os.write((char *) &item, sizeof(item));
    }
    // serialization of string
    static void serializeT(const std::string &s, std::ostream &os) {
      std::size_t size = s.size();
      os.write((char *) &size, sizeof(size));
      os.write(s.data(), size);
    }
    // serialization of CString
    static void serializeT(const CString &s, std::ostream &os) {
      std::size_t size = std::strlen(s.get());
      os.write((char *) &size, sizeof(size));
      os.write(s.get(), size);
    }

    // deserialization of numeric types
    template<typename T>
    static void deserializeT(T &item, std::ifstream &fs) {
      fs.read((char*)&item, sizeof(item));
    }
    // deserialization of string
    static void deserializeT(std::string &s, std::ifstream &fs) {
      std::size_t size;
      fs.read((char*) &size, sizeof(size));
      std::vector<char> v(size);
      fs.read(v.data(), size);
      s.append(v.begin(), v.end());
    }
    // deserialization of CString. Allocates memory but doesn't frees it.
    static void deserializeT(CString &str, std::ifstream &fs) {
      std::size_t size;
      fs.read((char*) &size, sizeof(size));
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

  // this code was inspired by
  // https://en.cppreference.com/w/cpp/utility/tuple/tuple_cat
    // helper functions to iterate over tuple
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
    };

   public:

    // prints tuple
    template<typename Row>
    static void print(const Row &row, std::ostream &os) {
      os << "(";
      TupleVisitor<Row, std::tuple_size<Row>::value>::print(row, os);
      os << ")" << std::endl;
    }

    // serializes tuple
    template<typename Row>
    static void serialize(const Row &row, std::ofstream &fs) {
      TupleVisitor<Row, std::tuple_size<Row>::value>::serialize(row, fs);
    }

    // deserializes tuple
    template<typename Row>
    static void deserialize(Row &row, std::ifstream &fs) {
      TupleVisitor<Row, std::tuple_size<Row>::value>::deserialize(row, fs);
    }

    // searches for CString type and frees allocated memory
    template<typename Row>
    static void cleanup(const Row &row) {
      TupleVisitor<Row, std::tuple_size<Row>::value>::cleanup(row);
    }
  };

  template<typename Row>
  class Table {
    std::vector<Row> table;
  };
}
