#ifndef READDICOM_H
#define READDICOM_H

#include <QString>
#include <math.h>
#include "volume.h"
#include "Types.h"

// class receives a list of DICOM files and creates volumes from them
class ReadDicom
{ 
public:
  ReadDicom(std::vector<QString> filenames);

  std::vector<ScalarVolume*> *getVolume();
  bool save(Volume *vol);

private:
  std::vector<QString> filenames;
  unsigned char *readSlice(QString filename, int &width, int &height,
                           int &channelPixelSize, float &sliceLocationOut,
                           QString &patientNameOut, QString &seriesDescriptionOut);
};

#endif // READDICOM_H
