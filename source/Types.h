#ifndef TYPES_H
#define TYPES_H

#include <list>
#include <boost/dynamic_bitset.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#define MIN2(a,b)     ((b)<(a)?(b):(a))
#define MAX2(a,b)     ((b)>(a)?(b):(a))
#define CLAMP(x,min,max) (MIN2(MAX2(min,x),max))

static int brushShapePixel[4] = {1, 4, 5, 12};
static int brushShape1[ 2] = { 0,0 };
static int brushShape2[ 8] = { 0,0 , 0,1 , 1,0 , 1,1 };
static int brushShape3[10] = { 0,0 , 0,1 , 0,-1 , 1,0 , -1,0 };
static int brushShape4[24] = { 0,0 , 0,1 , 0,2 , 0,-1 , 1,-1 , 1,0 , 1,1 , 1,2 , -1,0 , -1,1 , 2,0 , 2,1 };

typedef boost::geometry::model::d2::point_xy<double> point_type;
typedef boost::geometry::model::polygon<point_type> polygon_type;

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
