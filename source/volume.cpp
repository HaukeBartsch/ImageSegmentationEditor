#include <QSettings>
#include "volume.h"
#include "readmgz.h"

ScalarVolume::ScalarVolume()
{
  // create an empty volume
  this->range.resize(2,0);
  this->voxelsize.resize(3,1.0);
}
ScalarVolume::ScalarVolume(std::vector<int> dims, MyPrimType type)
{
    this->elementLength = 1; // default scalar field

    materialNames.resize(1);
    materialNames[0] = QString("Exterior");
    materialColors.resize(1);
    materialColors[0] = new QColor(0,0,0);
    filename = QString("empty");

    // create an empty volume
    this->size = dims;
    this->dataType = type;
    this->range.resize(2,0);
    size_t s = 1;
    for(std::vector<int>::iterator it = this->size.begin(); it != this->size.end(); ++it) {
        s *= *it;
    }
    this->dataPtr = (unsigned char *)malloc(s * MyPrimType::getTypeSize(this->dataType));
    if (!this->dataPtr) {
        fprintf(stderr, "Error reading file, could not get enough memory...");
    }
    this->voxelsize.resize(3,1.0);
}

Volume *ScalarVolume::duplicate() {
  ScalarVolume *ret = new ScalarVolume(this->size, this->dataType);
  ret->elementLength = this->elementLength;
  ret->autoWindowLevel[0] = this->autoWindowLevel[0];
  ret->autoWindowLevel[1] = this->autoWindowLevel[1];
  memcpy(ret->dataPtr,
         this->dataPtr,
         (size_t)this->elementLength*
         this->size[0]*
         this->size[1]*
         this->size[2]*
         MyPrimType::getTypeSize(this->dataType));
  ret->currentWindowLevel[0] = this->currentWindowLevel[0];
  ret->currentWindowLevel[1] = this->currentWindowLevel[1];
  ret->filename = this->filename;
  ret->loadCmd = this->loadCmd;
  ret->materialNames = this->materialNames;
  ret->materialColors = this->materialColors;
  ret->message = this->message;
  ret->range = this->range;
  ret->hist = this->hist;
  return (Volume *)ret;
}

ScalarVolume::~ScalarVolume() {
  if (this->dataPtr)
    free(this->dataPtr);
}

void ScalarVolume::saveAs( QString fileName ) {
  ReadMGZ *reader = new ReadMGZ(fileName);
  reader->save(this);
}

void ScalarVolume::updateRange() {
    if (this->size.size() == 0) {
        return;
    }
    if (this->size.size() != 3) {
        fprintf(stderr, "Warning: file has more than 3 dimensions, range is not correct");
    }
    range.resize(2); // min/max
    switch (this->dataType) {
    case MyPrimType::CHAR :
    case MyPrimType::UCHAR : {
          range[0] = range[1] = (double)this->dataPtr[0];
          unsigned char *d = this->dataPtr;
          for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
              if ( d[0] < range[0])
                  range[0] = (double)d[0];
              if ( d[0] > range[1])
                  range[1] = (double)d[0];
              d++;
          }
          break;
    }
    case MyPrimType::SHORT : {
        range[0] = range[1] = (double)this->dataPtr[0];
        short *d = (short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
            if ( d[0] < range[0])
                range[0] = (double)d[0];
            if ( d[0] > range[1])
                range[1] = (double)d[0];
            d++;
        }
        break;
    }
    case MyPrimType::USHORT : {
        range[0] = range[1] = (double)this->dataPtr[0];
        unsigned short *d = (unsigned short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2] ; i++) {
            if ( d[0] < range[0])
                range[0] = (double)d[0];
            if ( d[0] > range[1])
                range[1] = (double)d[0];
            d++;
        }
        break;
    }
    case MyPrimType::INT : {
        range[0] = range[1] = (double)this->dataPtr[0];
        int *d = (int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2] ; i++) {
            if ( d[0] < range[0])
                range[0] = (double)d[0];
            if ( d[0] > range[1])
                range[1] = (double)d[0];
            d++;
        }
        break;
    }
    case MyPrimType::UINT : {
        range[0] = range[1] = (double)this->dataPtr[0];
        unsigned int *d = (unsigned int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
            if ( d[0] < range[0])
                range[0] = (double)d[0];
            if ( d[0] > range[1])
                range[1] = (double)d[0];
            d++;
        }
        break;
    }
    case MyPrimType::FLOAT : {
        range[0] = range[1] = (double)this->dataPtr[0];
        float *d = (float *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
            if ( d[0] < range[0])
                range[0] = (double)d[0];
            if ( d[0] > range[1])
                range[1] = (double)d[0];
            d++;
        }
        break;
    }
    }

}

void ScalarVolume::computeHist() {
  hist.resize(512, 0);
  std::fill(hist.begin(),hist.end(),0);

  // range needs to be known before
  if (range.size() != 2)
    return;
  if (range[0] == range[1])
    return;

  switch (this->dataType) {
    case MyPrimType::CHAR : {
        signed char *d = (signed char *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()), 0, hist.size()-1)]++;
          d++;
        }
        break;
      }
    case MyPrimType::UCHAR : {
        unsigned char *d = (unsigned char *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()), 0, hist.size()-1)]++;
          d++;
        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *d = (signed short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()), 0, hist.size()-1)]++;
          d++;
        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *d = (unsigned short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()), 0, hist.size()-1)]++;
          d++;
        }
        break;
      }
    case MyPrimType::INT : {
        signed int *d = (signed int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()), 0, hist.size()-1)]++;
          d++;
        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *d = (unsigned int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()), 0, hist.size()-1)]++;
          d++;
        }
        break;
      }
    case MyPrimType::FLOAT : {
        float *d = (float *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()), 0, hist.size()-1)]++;
          d++;
        }
        break;
      }
  }
  // set window level to 2 and 99 percent
  long long cumsum = 0;
  for (unsigned int i = 0; i < hist.size(); i++) {
    cumsum += hist[i];
  }
  int step0 = 0; bool fs0 = false;
  int step1 = 0; bool fs1 = false;

  QSettings settings;
  float lowValue  = settings.value("Preferences/colorLowValue", 0.05).toDouble();
  float highValue = settings.value("Preferences/colorHighValue", 0.99).toDouble();

  double val = 0.0;
  for (unsigned int i = 0; i < hist.size(); i++) {
    val += (double)hist[i]/(double)cumsum;
    if (!fs0 && val > lowValue) {
      fs0 = true;
      step0 = i;
    }
    if (!fs1 && val > highValue) {
      fs1 = true;
      step1 = i;
    }
  }
  autoWindowLevel[0] = range[0] + ((step0-0.0)/hist.size()) * (range[1]-range[0]); // set initial window level by 2..99 percent
  autoWindowLevel[1] = range[0] + ((step1-0.0)/hist.size()) * (range[1]-range[0]);
}

ColorVolume *ScalarVolume::convertToColorVolume(ScalarVolume *red,
                                           ScalarVolume *green,
                                           ScalarVolume *blue,
                                           ScalarVolume *alpha) {
  ColorVolume *out = new ColorVolume(red->size, red->dataType);
  out->filename = red->filename;
  if (!out) {
    fprintf(stderr, "Error: not enough memory to create color object");
    return NULL;
  }
  switch (red->dataType) {
    case MyPrimType::UCHAR : {
        // copy components over
        unsigned char *o  = (unsigned char *)(out->dataPtr);
        unsigned char *cr = (unsigned char *)(red->dataPtr);
        unsigned char *cg = (unsigned char *)(green->dataPtr);
        unsigned char *cb = (unsigned char *)(blue->dataPtr);
        unsigned char *ca = NULL;
        if (alpha != NULL)
          ca = (unsigned char *)(alpha->dataPtr);

        for (ulong i = 0; i < (ulong)red->size[0] * red->size[1] * red->size[2]; i++) {
          o[0] = cr[0];
          //if (cr[0] > 0 || cg[0] > 0 || cb[0] > 0)
          //  fprintf(stderr, "%d %d %d\n", cr[0], cg[0], cb[0]);
          o[1] = cg[0];
          o[2] = cb[0];
          if (alpha != NULL) {
            o[3] = ca[0];
            ca++;
          } else {
            o[3] = 128;
          }
          cr++; cg++; cb++; o+=4;
        }
        break;
      }
    case MyPrimType::CHAR : {
        // copy components over
        signed char *o  = (signed char *)out->dataPtr;
        signed char *cr = (signed char *)red->dataPtr;
        signed char *cg = (signed char *)green->dataPtr;
        signed char *cb = (signed char *)blue->dataPtr;
        signed char *ca = NULL;
        if (alpha != NULL)
          ca = (signed char *)alpha->dataPtr;

        for (ulong i = 0; i < (ulong)red->size[0] * red->size[1] * red->size[2]; i++) {
          o[0] = cr[0];
          o[1] = cg[0];
          o[2] = cb[0];
          if (alpha != NULL) {
            o[3] = ca[0];
            ca++;
          } else {
            o[3] = 0;
          }
          cr++; cg++; cb++; o+=4;
        }
        break;
      }
    case MyPrimType::SHORT : {
        // copy components over
        signed short *o  = (signed short *)out->dataPtr;
        signed short *cr = (signed short *)red->dataPtr;
        signed short *cg = (signed short *)green->dataPtr;
        signed short *cb = (signed short *)blue->dataPtr;
        signed short *ca = NULL;
        if (alpha != NULL)
          ca = (signed short *)alpha->dataPtr;

        for (ulong i = 0; i < (ulong)red->size[0] * red->size[1] * red->size[2]; i++) {
          o[0] = cr[0];
          o[1] = cg[0];
          o[2] = cb[0];
          if (alpha != NULL) {
            o[3] = ca[0];
            ca++;
          } else {
            o[3] = 0;
          }
          cr++; cg++; cb++; o+=4;
        }
        break;
      }
    case MyPrimType::USHORT : {
        // copy components over
        unsigned short *o  = (unsigned short *)out->dataPtr;
        unsigned short *cr = (unsigned short *)red->dataPtr;
        unsigned short *cg = (unsigned short *)green->dataPtr;
        unsigned short *cb = (unsigned short *)blue->dataPtr;
        unsigned short *ca = NULL;
        if (alpha != NULL)
          ca = (unsigned short *)alpha->dataPtr;

        for (ulong i = 0; i < (ulong)red->size[0] * red->size[1] * red->size[2]; i++) {
          o[0] = cr[0];
          o[1] = cg[0];
          o[2] = cb[0];
          if (alpha != NULL) {
            o[3] = ca[0];
            ca++;
          } else {
            o[3] = 0;
          }
          cr++; cg++; cb++; o+=4;
        }
        break;
      }
    case MyPrimType::UINT : {
        // copy components over
        unsigned int *o  = (unsigned int *)out->dataPtr;
        unsigned int *cr = (unsigned int *)red->dataPtr;
        unsigned int *cg = (unsigned int *)green->dataPtr;
        unsigned int *cb = (unsigned int *)blue->dataPtr;
        unsigned int *ca = NULL;
        if (alpha != NULL)
          ca = (unsigned int *)alpha->dataPtr;

        for (ulong i = 0; i < (ulong)red->size[0] * red->size[1] * red->size[2]; i++) {
          o[0] = cr[0];
          o[1] = cg[0];
          o[2] = cb[0];
          if (alpha != NULL) {
            o[3] = ca[0];
            ca++;
          } else {
            o[3] = 0;
          }
          cr++; cg++; cb++; o+=4;
        }
        break;
      }
    case MyPrimType::INT : {
        // copy components over
        signed int *o  = (signed int *)out->dataPtr;
        signed int *cr = (signed int *)red->dataPtr;
        signed int *cg = (signed int *)green->dataPtr;
        signed int *cb = (signed int *)blue->dataPtr;
        signed int *ca = NULL;
        if (alpha != NULL)
          ca = (signed int *)alpha->dataPtr;

        for (ulong i = 0; i < (ulong)red->size[0] * red->size[1] * red->size[2]; i++) {
          o[0] = cr[0];
          o[1] = cg[0];
          o[2] = cb[0];
          if (alpha != NULL) {
            o[3] = ca[0];
            ca++;
          } else {
            o[3] = 0;
          }
          cr++; cg++; cb++; o+=4;
        }
        break;
      }
    case MyPrimType::FLOAT : {
        // copy components over
        float *o  = (float *)out->dataPtr;
        float *cr = (float *)red->dataPtr;
        float *cg = (float *)green->dataPtr;
        float *cb = (float *)blue->dataPtr;
        float *ca = NULL;
        if (alpha != NULL)
          ca = (float *)alpha->dataPtr;

        for (ulong i = 0; i < (ulong)red->size[0] * red->size[1] * red->size[2]; i++) {
          o[0] = cr[0];
          o[1] = cg[0];
          o[2] = cb[0];
          if (alpha != NULL) {
            o[3] = ca[0];
            ca++;
          } else {
            o[3] = 0;
          }
          cr++; cg++; cb++; o+=4;
        }
        break;
      }
  }

  out->updateRange();
  out->computeHist();
  out->currentWindowLevel[0] = out->autoWindowLevel[0];
  out->currentWindowLevel[1] = out->autoWindowLevel[1];
  out->filename = red->filename;
  out->message  = red->message;
  out->loadCmd  = red->loadCmd;

  return out;
}


// flip a dimension
void ScalarVolume::flip(int which) {
  // to be implememted
  which++;
}
// swap two dimensions
void ScalarVolume::swap(int which1, int which2) {
  // to be implemented
  which1++; which2++;
}


///////////////////////////////////////////////
// COLOR VOLUME CODE
///////////////////////////////////////////////

ColorVolume::ColorVolume()
{
  // create an empty volume
  this->range.resize(2,0);
  this->voxelsize.resize(3,1.0);
}
ColorVolume::ColorVolume(std::vector<int> dims, MyPrimType type)
{
    this->elementLength = 4; // default color field

    // it does not really makes sense to have a material section in here
    materialNames.resize(1);
    materialNames[0] = QString("Exterior");
    materialColors.resize(1);
    materialColors[0] = new QColor(0,0,0);

    // create an empty volume
    this->size = dims;
    this->dataType = type;
    this->range.resize(2,0);
    size_t s = 1;
    for(std::vector<int>::iterator it = this->size.begin(); it != this->size.end(); ++it) {
        s *= *it;
    }
    this->dataPtr = (unsigned char *)malloc(s * MyPrimType::getTypeSize(this->dataType) * this->elementLength);
    if (!this->dataPtr) {
        fprintf(stderr, "Error reading file, could not get enough memory...");
    }
    this->voxelsize.resize(3,1.0);
}

Volume *ColorVolume::duplicate() {
  ColorVolume *ret = new ColorVolume(this->size, this->dataType);
  ret->elementLength = this->elementLength;
  ret->autoWindowLevel[0] = this->autoWindowLevel[0];
  ret->autoWindowLevel[1] = this->autoWindowLevel[1];
  memcpy(ret->dataPtr,
         this->dataPtr,
         this->elementLength*
         this->size[0]*
         this->size[1]*
         this->size[2]*
         MyPrimType::getTypeSize(this->dataType));
  ret->currentWindowLevel[0] = this->currentWindowLevel[0];
  ret->currentWindowLevel[1] = this->currentWindowLevel[1];
  ret->filename = this->filename;
  ret->loadCmd = this->loadCmd;
  ret->materialNames = this->materialNames;
  ret->materialColors = this->materialColors;
  ret->message = this->message;
  ret->range = this->range;
  ret->hist = this->hist;
  return (Volume *)ret;
}

ColorVolume::~ColorVolume() {
  if (this->dataPtr)
    free(this->dataPtr);
}

float ColorVolume::getLuminance(float red, float green, float blue) {
    return 0.6*red + 0.2*green + 0.2*blue;
}

void ColorVolume::updateRange() {
    if (this->size.size() == 0) {
        return;
    }
    if (this->size.size() != 3) {
        fprintf(stderr, "Warning: file has more than 3 dimensions, range is not correct");
    }
    if (this->elementLength != 4) {
      fprintf(stderr, "Error: can only have four component structure for color");
      return;
    }
    range.resize(6); // min/max red, min/max green, min/max blue
    switch (this->dataType) {
      case MyPrimType::CHAR :
      case MyPrimType::UCHAR : {
          range[0] = range[1] = range[2] = range[3] = range[4] = range[5] = (double)this->dataPtr[0];
          unsigned char *d = this->dataPtr;
          for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
            if ( d[0] < range[0])
              range[0] = (double)d[0];
            if ( d[0] > range[1])
              range[1] = (double)d[0];
            if ( d[1] < range[2])
              range[2] = (double)d[1];
            if ( d[1] > range[3])
              range[3] = (double)d[1];
            if ( d[2] < range[4])
              range[4] = (double)d[2];
            if ( d[2] > range[5])
              range[5] = (double)d[2];
            d+=4;
          }
          break;
        }
    case MyPrimType::SHORT : {
        range[0] = range[1] = range[2] = range[3] = range[4] = range[5] = (double)this->dataPtr[0];
        short *d = (short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          if ( d[0] < range[0])
            range[0] = (double)d[0];
          if ( d[0] > range[1])
            range[1] = (double)d[0];
          if ( d[1] < range[2])
            range[2] = (double)d[1];
          if ( d[1] > range[3])
            range[3] = (double)d[1];
          if ( d[2] < range[4])
            range[4] = (double)d[2];
          if ( d[2] > range[5])
            range[5] = (double)d[2];
          d+=4;
        }
        break;
    }
    case MyPrimType::USHORT : {
        range[0] = range[1] = range[2] = range[3] = range[4] = range[5] = (double)this->dataPtr[0];
        unsigned short *d = (unsigned short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2] ; i++) {
          if ( d[0] < range[0])
            range[0] = (double)d[0];
          if ( d[0] > range[1])
            range[1] = (double)d[0];
          if ( d[1] < range[2])
            range[2] = (double)d[1];
          if ( d[1] > range[3])
            range[3] = (double)d[1];
          if ( d[2] < range[4])
            range[4] = (double)d[2];
          if ( d[2] > range[5])
            range[5] = (double)d[2];
          d+=4;
        }
        break;
    }
    case MyPrimType::INT : {
        range[0] = range[1] = range[2] = range[3] = range[4] = range[5] = (double)this->dataPtr[0];
        int *d = (int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2] ; i++) {
          if ( d[0] < range[0])
            range[0] = (double)d[0];
          if ( d[0] > range[1])
            range[1] = (double)d[0];
          if ( d[1] < range[2])
            range[2] = (double)d[1];
          if ( d[1] > range[3])
            range[3] = (double)d[1];
          if ( d[2] < range[4])
            range[4] = (double)d[2];
          if ( d[2] > range[5])
            range[5] = (double)d[2];
          d+=4;
        }
        break;
    }
    case MyPrimType::UINT : {
        range[0] = range[1] = range[2] = range[3] = range[4] = range[5] = (double)this->dataPtr[0];
        unsigned int *d = (unsigned int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          if ( d[0] < range[0])
            range[0] = (double)d[0];
          if ( d[0] > range[1])
            range[1] = (double)d[0];
          if ( d[1] < range[2])
            range[2] = (double)d[1];
          if ( d[1] > range[3])
            range[3] = (double)d[1];
          if ( d[2] < range[4])
            range[4] = (double)d[2];
          if ( d[2] > range[5])
            range[5] = (double)d[2];
          d+=4;
        }
        break;
    }
    case MyPrimType::FLOAT : {
        range[0] = range[1] = range[2] = range[3] = range[4] = range[5] = (double)this->dataPtr[0];
        float *d = (float *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          if ( d[0] < range[0])
            range[0] = (double)d[0];
          if ( d[0] > range[1])
            range[1] = (double)d[0];
          if ( d[1] < range[2])
            range[2] = (double)d[1];
          if ( d[1] > range[3])
            range[3] = (double)d[1];
          if ( d[2] < range[4])
            range[4] = (double)d[2];
          if ( d[2] > range[5])
            range[5] = (double)d[2];
          d+=4;
        }
        break;
    }
    }

}

void ColorVolume::computeHist() {
  hist.resize(512*elementLength, 0);

  // range needs to be known before
  if (range.size() != 6) {
    fprintf(stderr, "Cannot compute histogram before range");
    return;
  }
  if (range[0] == range[1] && range[2] == range[3] && range[4] == range[5]) {
    fprintf(stderr, "Warning: empty data range, nothing to do");
    return;
  }

  switch (this->dataType) {
    case MyPrimType::CHAR : {
        signed char *d = (signed char *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 0*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[1]-range[2])/(range[3]-range[2]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 1*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[2]-range[4])/(range[5]-range[4]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 2*hist.size()/elementLength]++;
          d+=4;
        }
        break;
      }
    case MyPrimType::UCHAR : {
        unsigned char *d = (unsigned char *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 0*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[1]-range[2])/(range[3]-range[2]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 1*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[2]-range[4])/(range[5]-range[4]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 2*hist.size()/elementLength]++;
          d+=4;
        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *d = (signed short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 0*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[1]-range[2])/(range[3]-range[2]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 1*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[2]-range[4])/(range[5]-range[4]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 2*hist.size()/elementLength]++;
          d+=4;
        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *d = (unsigned short *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 0*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[1]-range[2])/(range[3]-range[2]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 1*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[2]-range[4])/(range[5]-range[4]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 2*hist.size()/elementLength]++;
          d+=4;
        }
        break;
      }
    case MyPrimType::INT : {
        signed int *d = (signed int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 0*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[1]-range[2])/(range[3]-range[2]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 1*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[2]-range[4])/(range[5]-range[4]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 2*hist.size()/elementLength]++;
          d+=4;
        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *d = (unsigned int *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 0*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[1]-range[2])/(range[3]-range[2]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 1*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[2]-range[4])/(range[5]-range[4]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 2*hist.size()/elementLength]++;
          d+=4;
        }
        break;
      }
    case MyPrimType::FLOAT : {
        float *d = (float *)this->dataPtr;
        for (long i = 0; i < (long)this->size[0] * this->size[1] * this->size[2]; i++) {
          hist[CLAMP( (unsigned int) ( (d[0]-range[0])/(range[1]-range[0]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 0*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[1]-range[2])/(range[3]-range[2]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 1*hist.size()/elementLength]++;
          hist[CLAMP( (unsigned int) ( (d[2]-range[4])/(range[5]-range[4]) * hist.size()/elementLength), 0, hist.size()/elementLength-1) + 2*hist.size()/elementLength]++;
          d+=4;
        }
        break;
      }
  }
  // set window level to 2 and 99 percent
  long long cumsum1 = 0;
  long long cumsum2 = 0;
  long long cumsum3 = 0;
  for (unsigned int i = 0; i < hist.size()/elementLength; i++) {
    cumsum1 += hist[i];
    cumsum2 += hist[i + 1*hist.size()/elementLength];
    cumsum3 += hist[i + 2*hist.size()/elementLength];
  }
  int step0 = 0; bool fs0 = false;
  int step1 = 0; bool fs1 = false;
  double val = 0.0;

  QSettings settings;
  float lowValue = settings.value("Preferences/colorLowValue", 0.05).toDouble();
  float highValue = settings.value("Preferences/colorHighValue", 0.9999).toDouble();

  for (unsigned int i = 0; i < hist.size()/elementLength; i++) {
    val += (double)hist[i]/(double)cumsum1;
    if (!fs0 && val > lowValue) {
      fs0 = true;
      step0 = i;
    }
    if (!fs1 && val > highValue) {
      fs1 = true;
      step1 = i;
    }
  }
  float rangeCombined[2];
  rangeCombined[0] = (range[0] + range[2] + range[4])/(float)elementLength;
  rangeCombined[1] = (range[1] + range[3] + range[5])/(float)elementLength;
  autoWindowLevel[0] = rangeCombined[0] + ((step0-0.0)/(hist.size()/(float)elementLength)) * (rangeCombined[1]-rangeCombined[0]); // set initial window level by 2..99 percent
  autoWindowLevel[1] = rangeCombined[0] + ((step1-0.0)/(hist.size()/(float)elementLength)) * (rangeCombined[1]-rangeCombined[0]);
}

// flip a dimension
void ColorVolume::flip(int which) {
  // to be implememted
  which++;
}

// swap two dimensions
void ColorVolume::swap(int which1, int which2) {
  // to be implemented
  which1++; which2++;
}
