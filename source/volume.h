#ifndef VOLUME_H
#define VOLUME_H

#include <QColor>
#include <QString>
#include <vector>
#include "Types.h"

class ColorVolume;

class Volume {
public:
  int elementLength;   // 1 for gray-scale, 4 for color volume

  // store material information
  std::vector<QString> materialNames;
  std::vector<QColor*> materialColors;
  QString loadCmd;
  QString filename;
  QString message;
  unsigned char *dataPtr; // pointer to real data
  MyPrimType dataType;    // one of Types data types

  std::vector<double> voxelsize;
  std::vector<int> size;
  std::vector<double> range;
  float autoWindowLevel[2];    // automatic window level computed by computeHist()
  float currentWindowLevel[2]; // currently used window level for this volume
  std::vector<long> hist;

  virtual void updateRange() = 0;
  virtual void computeHist() = 0;
  virtual Volume *duplicate() = 0;
};

class ScalarVolume : public Volume
{
public:
    ScalarVolume();
    ScalarVolume(std::vector<int> dims, MyPrimType type);
    ~ScalarVolume();

    void flip(int which);
    void swap(int which1, int which2);

    virtual void updateRange();
    virtual void computeHist();
    virtual Volume *duplicate();

    ColorVolume *convertToColorVolume(ScalarVolume *red, ScalarVolume *green, ScalarVolume *blue, ScalarVolume *alpha = NULL);

    void saveAs(QString fileName);
};

class ColorVolume : public Volume
{
public:
    ColorVolume();
    ColorVolume(std::vector<int> dims, MyPrimType type);
    ~ColorVolume();

    void flip(int which);
    void swap(int which1, int which2);

    float getLuminance(float red, float green, float blue);

    virtual void updateRange();
    virtual void computeHist();
    virtual Volume *duplicate();

    // store bounding box information
};


#endif // VOLUME_H
