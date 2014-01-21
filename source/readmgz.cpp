#include <QMessageBox>
#include "readmgz.h"

#include <qfileinfo>
#include <QTemporaryFile>
#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

struct endHeader {
        float tr;
        float flipAngle;
        float te;
        float ti;
        float FoV;
        // followed by tags (variable length string)
};


ReadMGZ::ReadMGZ(QString filename)
{
   this->filename = filename;
}


void ReadMGZ::swapBytesWord4(void* dp, ulong nn)
{
  int *d = (int*) dp;
  for (ulong i=0 ; i<nn ; i++) {
    *d = ((*d & 0xff) << 24) | ((*d & 0xff00)<<8) |
      ((*d & 0xff0000) >>8) | ((*d & 0xff000000)>>24);
    d++;
  }
}

void ReadMGZ::swapBytesWord2(void* dp, ulong nn)
{
    unsigned short *d = (unsigned short*) dp;
    for (ulong i=0 ; i<nn ; i++) {
      *d = ((*d & 0xff) << 8) | ((*d & 0xff00)>>8);
      d++;
    }
}

// save using the filename specified in the constructor
bool ReadMGZ::save(Volume *vol) {

  FILE* fp = fopen(this->filename.toLatin1().constData(),"wb");

  if (!fp) {
      fprintf(stderr, "Error: could not open file for writing...");
      return false;
  }

  // write the first part of the header for MGH
  int version = 1; // should be 1
  int width   = vol->size[0];   // fastest running index (x)
  int height  = vol->size[1];   // y
  int depth   = vol->size[2];   // z
  int nframes = 1; // number of components per pixel
  int type    = 0; // uchar, short, int, float (0,4,1, or 3)
  if (vol->dataType == MyPrimType::UCHAR) {
      type = 0;
  } else if (vol->dataType == MyPrimType::SHORT) {
      type = 4;
  } else if (vol->dataType == MyPrimType::INT) {
      type = 1;
  } else if (vol->dataType == MyPrimType::FLOAT) {
      type = 3;
  } else {
      fprintf(stderr, "Error: data type not supported, only unsigned char, short, int, and float are.");
      return false;
  }

  int dof = 0;     // degrees of freedom (ignore)
  short goodRASFlag = 1; // if true use direction cosines, else use coronal

  int bc = 0; // byte counter
  writeInt4(version, fp);     bc+=4;
  writeInt4(width, fp);       bc+=4;
  writeInt4(height, fp);      bc+=4;
  writeInt4(depth, fp);       bc+=4;
  writeInt4(nframes, fp);     bc+=4;
  writeInt4(type, fp);        bc+=4;
  writeInt4(dof, fp);         bc+=4;
  writeInt2(goodRASFlag, fp); bc+=2;

  if (vol->voxelsize.size() != 3) {
    fprintf(stderr, "Error: voxel size is not defined for this volume");
    return false;
  }
  float vz[3]; vz[0] = vol->voxelsize[0]; vz[1] = vol->voxelsize[1]; vz[2] = vol->voxelsize[2];
  float Mdc[9]; memset(Mdc, 0, sizeof(float)*6);
  Mdc[0]  = 1;
  Mdc[4]  = 1;
  Mdc[8] = 1;
  float Pxyz[3]; memset(Pxyz, 0, sizeof(float)*3);
  if (goodRASFlag) {
      swapBytesWord4((void *)&vz[0], 3); fwrite(vz, 3, sizeof(float), fp); bc+=3*sizeof(float);     // fread(vz,  sizeof(float), 3, fp);
      swapBytesWord4((void *)&Mdc[0], 9); fwrite(Mdc, 9, sizeof(float), fp); bc+=9*sizeof(float);   // fread(Mdc, sizeof(float), 9, fp);
      swapBytesWord4((void *)&Pxyz[0], 3); fwrite(Pxyz, 3, sizeof(float), fp); bc+=3*sizeof(float); // fread(Pxyz, sizeof(float), 3, fp);
      //theMsg->printf("Pxyz is %g %g %g", Pxyz[0], Pxyz[1], Pxyz[2]);
  }
  // advance to 284
  char b[] = { 0 };
  fwrite(b, 284-bc, 1, fp);

  ulong nvals = (ulong)vol->size[0]*vol->size[1]*vol->size[2];
  if (vol->dataType == MyPrimType::UCHAR) {
      fwrite(vol->dataPtr, sizeof(unsigned char), nvals, fp);
  } else if (vol->dataType == MyPrimType::SHORT) {
      signed short *buffer = (signed short *)malloc(sizeof(signed short)*nvals);
      if (!buffer) {
          fprintf(stderr, "Error: not enough memory.");
          return 0;
      }
      memcpy(buffer, vol->dataPtr, sizeof(signed short)*nvals);
      swapBytesWord2(buffer, nvals);
      fwrite(buffer, sizeof(signed short), nvals, fp);
      free(buffer);
  } else if (vol->dataType == MyPrimType::INT) {
      signed int *buffer = (signed int *)malloc(sizeof(signed int)*nvals);
      if (!buffer) {
          fprintf(stderr, "Error: not enough memory.");
          return 0;
      }
      memcpy(buffer, vol->dataPtr, sizeof(signed int)*nvals);
      swapBytesWord4(buffer, nvals);
      fwrite(buffer, sizeof(signed int), nvals, fp);
      free(buffer);
  } else if (vol->dataType == MyPrimType::FLOAT) {
      float *buffer = (float *)malloc(sizeof(float)*nvals);
      if (!buffer) {
          fprintf(stderr, "Error: not enough memory.");
          return 0;
      }
      memcpy(buffer, vol->dataPtr, sizeof(float)*nvals);
      swapBytesWord4(buffer, nvals);
      fwrite(buffer, sizeof(float), nvals, fp);
      free(buffer);
  } else {
      fprintf(stderr, "Error: wrong data type, not unsigned char, short, int, or float");
      return 0;
  }

  endHeader h2;
  // read these values from the MGH header section (todo)
  h2.flipAngle = 0;
  h2.FoV = 0;
  h2.te = 0;
  h2.ti = 0;
  h2.tr = 0;
  fwrite(&h2, 1, sizeof(endHeader), fp);
  fclose(fp);
  vol->filename = this->filename;

  return true;
}


void ReadMGZ::writeInt3(int value, FILE *fp) {
  unsigned int tmp = value;
  unsigned char b1 = ((tmp>>16) & 0x0000ff);
  unsigned char b2 = ((tmp>>8) & 0x0000ff);
  unsigned char b3 = (tmp & 0x0000ff);
  fwrite(&b1, 1, 1, fp);
  fwrite(&b2, 1, 1, fp);
  fwrite(&b3, 1, 1, fp);
}
void ReadMGZ::writeInt2(int value, FILE *fp) {
  unsigned int tmp = value;
  unsigned char b1 = ((tmp>>8) & 0x0000ff);
  unsigned char b2 = (tmp & 0x0000ff);
  fwrite(&b1, 1, 1, fp);
  fwrite(&b2, 1, 1, fp);
}

void ReadMGZ::writeInt4(int value, FILE *fp) {
  int tmp = value;
  swapBytesWord4(&tmp, 1);
  fwrite(&tmp, 4, 1, fp);
}


std::vector<ScalarVolume*> *ReadMGZ::getVolume() {

    QString fn = filename;

    // if the file is gzip compressed uncompress first
/*    QFileInfo name = QFileInfo(filename);
    if (name.suffix() == "mgz") {
      // uncompress first, lost uncompressed file instead
      QTemporaryFile file;
      QString tempFn;
      if (file.open()) {
        tempFn = file.fileName();
      }
      std::ifstream file1(filename.toLatin1().constData(),
                         std::ios_base::in | std::ios_base::binary);
      boost::iostreams::filtering_streambuf < boost::iostreams::input > in;
      in.push(boost::iostreams::gzip_decompressor());
      in.push(file1);

      std::ofstream file2(tempFn.toLatin1().constData(),
                         std::ios_base::out | std::ios_base::binary);

      boost::iostreams::copy(in, file2);

    } */

    FILE* fp = fopen(fn.toLatin1().constData(),"rb");

    if (!fp) {
      return NULL;
    }

    int version; // should be 1
    int width;   // fastest running index (x)
    int height;  // y
    int depth;   // z
    int nframes; // number of components per pixel
    int type;    // uchar, short, int, float (0,4,1, or 3)
    int dof;     // degrees of freedom (ignore)
    short goodRASFlag; // if true use direction cosines, else use coronal

    fread((void *)&version, sizeof(int), 1, fp); swapBytesWord4((void *)&version, 1);
    fread((void *)&width,   sizeof(int), 1, fp); swapBytesWord4((void *)&width, 1);
    fread((void *)&height,  sizeof(int), 1, fp); swapBytesWord4((void *)&height, 1);
    fread((void *)&depth,   sizeof(int), 1, fp); swapBytesWord4((void *)&depth, 1);
    fread((void *)&nframes, sizeof(int), 1, fp); swapBytesWord4((void *)&nframes, 1);
    fread((void *)&type,    sizeof(int), 1, fp); swapBytesWord4((void *)&type, 1);
    fread((void *)&dof,     sizeof(int), 1, fp); swapBytesWord4((void *)&dof, 1);
    fread((void *)&goodRASFlag, sizeof(short), 1, fp); swapBytesWord2((void *)&goodRASFlag, 1);

    if (version != 1) {
      fprintf(stderr, "Warning: Supported version of MGH file format is 1 (found %d instead)",
                    version);
    }

    std::vector<int> dims(3);
    dims[0] = width;
    dims[1] = height;
    dims[2] = depth;
    int bytesPerComp = 1;
    MyPrimType ty = MyPrimType(MyPrimType::CHAR);
    QString typeName("char");
    switch (type) {
    case 0:
      ty = MyPrimType::UCHAR;
      bytesPerComp = 1;
      typeName = "char";
      break;
    case 1:
      ty = MyPrimType::INT;
      bytesPerComp = 4;
      typeName = "int";
      break;
    case 2:
      fprintf(stderr, "Error: unknown type 2");
      break;
    case 3:
      ty = MyPrimType::FLOAT;
      bytesPerComp = 4;
      typeName = "float";
      break;
    case 4:
      ty = MyPrimType::SHORT;
      bytesPerComp = 2;
      typeName = "short";
      break;
    default:
      fprintf(stderr, "Error: unknow data type");
      return NULL;
    }

    float vz[3];
    float Mdc[9];
    float Pxyz[3];
    // McDVector<float> Pxyz_0(3);
    if (goodRASFlag) {
      fread(vz,  sizeof(float), 3, fp);  swapBytesWord4((void *)&vz[0], 3);
      fread(Mdc, sizeof(float), 9, fp);  swapBytesWord4((void *)&Mdc[0], 9);
      fread(Pxyz, sizeof(float), 3, fp); swapBytesWord4((void *)&Pxyz[0], 3);
      //theMsg->printf("Pxyz is %g %g %g", Pxyz[0], Pxyz[1], Pxyz[2]);
    }

    if (fabs(vz[0]) < 1e-6) {
        vz[0] = 1;
        fprintf(stderr, "Error: could not read voxel size, use default of 1");
    }
    if (fabs(vz[1]) < 1e-6)
        vz[1] = 1;
    if (fabs(vz[2]) < 1e-6)
        vz[2] = 1;

    fseek(fp, 284, SEEK_SET);

    int frames = nframes;
    std::vector<ScalarVolume*> *volumes = new std::vector<ScalarVolume*>();
    for (int i = 0; i < frames; i++) {
        ScalarVolume *data = new ScalarVolume(dims, ty);
        if (!data) {
            fprintf(stderr, "Error: data could not be created");
            continue;
        }
        //data->lattice.setBoundingBox(bbox);
        //data->setTransform(mat);
        data->filename = filename;

        QString name = QString("comp%1").arg(i);
        //data->composeLabel(McFilename(filename).basename(), name.getString() );
        size_t readBytes = fread(data->dataPtr, 1, bytesPerComp*(ulong)dims[0]*dims[1]*dims[2], fp);
        if (readBytes != bytesPerComp*(ulong)dims[0]*dims[1]*dims[2]) {
          fprintf(stderr, "ERROR: could not read all bytes (%fMB) from the file (%fMB)", (bytesPerComp*(ulong)dims[0]*dims[1]*dims[2])/1024.0/1024.0, readBytes/1024.0/1024.0);
          continue;
        }
        switch (type) {
        case 0:
            break;
        case 1:
        {
            int *d3 = (int *)data->dataPtr;
            swapBytesWord4((void *)d3, (ulong)dims[0]*dims[1]*dims[2]);
        }
            break;
        case 2:
            fprintf(stderr, "Error: unknown type");
            break;
        case 3:
        {
            float *d2 = (float *)data->dataPtr;
            swapBytesWord4((void *)d2, (ulong)dims[0]*dims[1]*dims[2]);
        }
            break;
        case 4:
        {
            short *d = (short *)data->dataPtr;
            swapBytesWord2((void *)d, (ulong)dims[0]*dims[1]*dims[2]);
        }
            break;
        default:
            fprintf(stderr, "Error: unknown type");
            break;
        }
        data->updateRange();
        for (int j = 1; j <= data->range[1]; j++) {
          data->materialNames.push_back(QString("material%1").arg(j));
          float saturation = 0.6;
          float lightness  = 0.75;
          float hue = (float)qrand() / RAND_MAX;
          hue += 0.618033988749895;
          hue = fmod(hue, 1.0);
          QColor color = QColor::fromHslF(hue, saturation, lightness);

          data->materialColors.push_back( new QColor(color) );
          data->materialVisibility.push_back( true );
        }

        data->computeHist();
        data->currentWindowLevel[0] = data->autoWindowLevel[0];
        data->currentWindowLevel[1] = data->autoWindowLevel[1];
        data->voxelsize.resize(3);
        data->voxelsize[0] = vz[0];
        data->voxelsize[1] = vz[1];
        data->voxelsize[2] = vz[2];
        volumes->push_back(data);
    }

    endHeader h2;
    fread(&h2, 1, sizeof(endHeader), fp);
    // find out how much more is in the file and allocate that much space for the tags
    long here = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long end = ftell(fp);
    fseek(fp, here, SEEK_SET);
    QString tagText("");
    if (end-here > 0) {
      unsigned char *tags = (unsigned char *)malloc(sizeof(unsigned char)*((end-here)+1));
      if (!tags) {
        fprintf(stderr, "Error: not enough memory to read tags in the file");
        return volumes;
      }
      fread(tags, end-here, sizeof(unsigned char), fp);
      tags[end-here+1] = '\0';
    }
    fclose(fp);
    for(unsigned int i = 0; i < volumes->size(); i++) {
      volumes->at(i)->message = tagText;
      volumes->at(i)->loadCmd = QString("read as mgh from %1").arg(filename.toLatin1().constData());
    }
    return volumes;
}
