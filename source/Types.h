#ifndef TYPES_H
#define TYPES_H

#define MIN2(a,b)     ((b)<(a)?(b):(a))
#define MAX2(a,b)     ((b)>(a)?(b):(a))
#define CLAMP(x,min,max) (MIN2(MAX2(min,x),max))

class MyPrimType {

public:
  // primitive data types for the raw data
  enum Type {
     CHAR = 0, UCHAR, SHORT, USHORT, INT, UINT, FLOAT
  };

  operator int() const {
      return type;
  }

  // returns size in bytes
  static int getTypeSize(MyPrimType type) {
      int t = type;
      switch(t) {
      case MyPrimType::CHAR :
      case MyPrimType::UCHAR : {
          return 1;
          break;
      };
      case MyPrimType::SHORT :
      case MyPrimType::USHORT : {
          return 2;
          break;
      };
      case MyPrimType::FLOAT :
      case MyPrimType::INT :
      case MyPrimType::UINT : {
          return 4;
          break;
      };
      }
      return 0;
  }

  MyPrimType() {
      type = CHAR;
  }

  MyPrimType(int type) {
      this->type = type;
  }

  int operator=(int other) {
      type = other;
      return type;
  }

protected:
  int type;
};

#endif // TYPES_H
