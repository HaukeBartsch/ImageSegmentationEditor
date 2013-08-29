#include "readdicom.h"

#include <iostream>

#include "imebra/imebra/include/imebra.h"
#include <sstream>

#ifdef PUNTOEXE_WINDOWS
#include <process.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#endif

#include <memory>
#include <list>
#include <boost/tuple/tuple.hpp>

using namespace puntoexe;
using namespace puntoexe::imebra;

class tupel;

ReadDicom::ReadDicom(std::vector<QString> filenames)
{
  this->filenames = filenames;
}

// always exports three channel images
unsigned char *ReadDicom::readSlice(QString filename, int &widthOut, int &heightOut,
                                    int &channelPixelSizeOut, float &sliceLocationOut,
                                    QString &patientNameOut, QString &seriesDescriptionOut) {

  unsigned char *buffer = NULL;
  size_t framesCount(0);
  ptr<dataSet> loadedDataSet;

  try
  {

      // Open the file containing the dicom dataset
      ptr<puntoexe::stream> inputStream(new puntoexe::stream);
      inputStream->openFile(filename.toStdString(), std::ios_base::in);

      // Connect a stream reader to the dicom stream. Several stream reader
      //  can share the same stream
      ptr<puntoexe::streamReader> reader(new streamReader(inputStream));

      // Get a codec factory and let it use the right codec to create a dataset
      //  from the input stream
      ptr<codecs::codecFactory> codecsFactory(codecs::codecFactory::getCodecFactory());
      loadedDataSet = codecsFactory->load(reader, 2048);


      // Get the first image. We use it in case there isn't any presentation VOI/LUT
      //  and we have to calculate the optimal one
      ptr<image> dataSetImage(loadedDataSet->getImage(0));
      imbxUint32 width, height;
      dataSetImage->getSize(&width, &height);
      widthOut = (int)width;
      heightOut = (int)height;

      // Build the transforms chain
      ptr<transforms::transformsChain> chain(new transforms::transformsChain);

      ptr<transforms::modalityVOILUT> modalityVOILUT(new transforms::modalityVOILUT(loadedDataSet));
      chain->addTransform(modalityVOILUT);

      ptr<transforms::colorTransforms::colorTransformsFactory> colorFactory(transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
      if (colorFactory->isMonochrome(dataSetImage->getColorSpace()))
      {
          // Convert to MONOCHROME2 if a modality transform is not present
          ////////////////////////////////////////////////////////////////
          if(modalityVOILUT->isEmpty())
          {
              ptr<transforms::colorTransforms::colorTransform> monochromeColorTransform(colorFactory->getTransform(dataSetImage->getColorSpace(), L"MONOCHROME2"));
              if(monochromeColorTransform != 0)
              {
                  chain->addTransform(monochromeColorTransform);
              }
          }

          ptr<transforms::VOILUT> presentationVOILUT(new transforms::VOILUT(loadedDataSet));
          imbxUint32 firstVOILUTID(presentationVOILUT->getVOILUTId(0));
          if(firstVOILUTID != 0)
          {
              presentationVOILUT->setVOILUT(firstVOILUTID);
          }
          else
          {
              // Run the transform on the first image
              ///////////////////////////////////////
              ptr<image> temporaryImage = chain->allocateOutputImage(dataSetImage, width, height);
              chain->runTransform(dataSetImage, 0, 0, width, height, temporaryImage, 0, 0);

              // Now find the optimal VOILUT
              //////////////////////////////
              presentationVOILUT->applyOptimalVOI(temporaryImage, 0, 0, width, height);
          }
          chain->addTransform(presentationVOILUT);
      }

      std::wstring initialColorSpace;
      if(chain->isEmpty())
      {
          initialColorSpace = dataSetImage->getColorSpace();
      }
      else
      {
          ptr<image> startImage(chain->allocateOutputImage(dataSetImage, 1, 1));
          initialColorSpace = startImage->getColorSpace();
      }

      // Color transform to YCrCb
      ptr<transforms::colorTransforms::colorTransform> colorTransform(colorFactory->getTransform(initialColorSpace, L"YBR_FULL"));
      if(colorTransform != 0)
      {
          chain->addTransform((colorTransform));
      }

      ptr<image> finalImage(new image);
      finalImage->create(width, height, image::depthU8, L"YBR_FULL", 7);

      // Scan through the frames
      for(imbxUint32 frameNumber(0); ; ++frameNumber)
      {
          if(frameNumber != 0)
          {
              dataSetImage = loadedDataSet->getImage(frameNumber);
          }


          if(chain->isEmpty() && dataSetImage->getDepth() != finalImage->getDepth() && dataSetImage->getHighBit() != finalImage->getHighBit())
          {
              chain->addTransform(new transforms::transformHighBit);
          }

          if(!chain->isEmpty())
          {
              chain->runTransform(dataSetImage, 0, 0, width, height, finalImage, 0, 0);
          }
          else
          {
              finalImage = dataSetImage;
          }

          // finalImage
          imbxUint32 pRowSize, pChannelPixelSize, pChannelsNumber;
          ptr<handlers::dataHandlerNumericBase> data = finalImage->getDataHandler(false,
                  &pRowSize,
                  &pChannelPixelSize,
                  &pChannelsNumber);
          channelPixelSizeOut = (int)pChannelPixelSize;
          if (buffer != NULL)
            free(buffer);
          buffer = (unsigned char *)malloc(sizeof(unsigned char) * width * height * pChannelPixelSize * pChannelsNumber);
          if (pChannelPixelSize == 1)
            data->copyTo( buffer, sizeof(unsigned char) * width * height * pChannelsNumber * pChannelPixelSize );
          else
            fprintf(stderr, "Error: pixel size for channel is not 1 but: %d\n", pChannelPixelSize);
          ++framesCount;
      }
  }
  catch(dataSetImageDoesntExist&)
  {
          // Ignore this exception. It is thrown when we reach the
          //  end of the images list
          exceptionsManager::getMessage();
  }

  // find a dicom tag:
  double frameTime(loadedDataSet->getDouble(0x0018, 0, 0x1063, 0));
  double sliceLocation(loadedDataSet->getDouble(0x0020, 0, 0x1041, 0));
  sliceLocationOut = sliceLocation;
  QString patientName = QString::fromStdString(loadedDataSet->getString(0x0010, 0, 0x0010, 0));
  QString seriesDescription = QString::fromStdString(loadedDataSet->getString(0x0008,0,0x103E,0));
  patientNameOut = patientName;
  seriesDescriptionOut = seriesDescription;

  return buffer;
}

bool sortSlices( boost::tuple<float, unsigned char*> x, boost::tuple<float, unsigned char*> y) {
  return (x.get_head() < y.get_head());
}

std::vector<ScalarVolume*> *ReadDicom::getVolume() {

    std::vector<ScalarVolume*> *volumes = new std::vector<ScalarVolume *>();

    // check all the slices, sort by slice location 0020,1041
    std::vector< boost::tuple<float, unsigned char *> > slices;
    int width, height, seriesWidth, seriesHeight, seriesChannels;
    QString patientName, seriesDescription;
    for (int i = 0; i < this->filenames.size(); i++) {
      int channels = 0;
      float sliceLocation = 0.0f;
      QString fn(filenames.at(i));
      unsigned char *b = readSlice(fn, width, height, channels, sliceLocation, patientName, seriesDescription );
      slices.push_back( boost::tuple<float, unsigned char *>(sliceLocation, b) );
      if (i == 0) {
         seriesWidth = width;
         seriesHeight = height;
         seriesChannels = channels;
      }
      if (width != seriesWidth) {
        fprintf(stderr, "Error: not all files have the same width (%d <-> %d)\n", seriesWidth, width);
        return NULL;
      }
      if (height != seriesHeight) {
        fprintf(stderr, "Error: not all files have the same height (%d <-> %d)\n", seriesHeight, height);
        return NULL;
      }
      if (channels != channels) {
        fprintf(stderr, "Error: not all files have the same number of channels");
        return NULL;
      }
    }

    // now sort by slice location (smallest to largest)
    std::sort( slices.begin(), slices.end(), sortSlices );

    // how many slices?
    int numSlices = slices.size();
    std::vector<int> dims;
    dims.push_back(width);
    dims.push_back(height);
    dims.push_back(numSlices);
    ScalarVolume *vol = new ScalarVolume(dims, MyPrimType::CHAR);

    int count = 0;
    for (std::vector< boost::tuple<float, unsigned char *> >::iterator it=slices.begin(); it!=slices.end(); ++it) {
      float sliceLocation = it->get<0>();
      unsigned char *data = it->get<1>();
      for ( int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
          vol->dataPtr[count*(width*height) + j*width + i] = data[3*(j*width+i)+0];
        }
      }
      free(data); // give the memory back for this buffer object

      fprintf(stderr, "slice location (%d): %f\n", count++, sliceLocation);
    }
    vol->filename = patientName + seriesDescription;
    vol->updateRange();
    vol->computeHist();
    vol->currentWindowLevel[0] = vol->autoWindowLevel[0];
    vol->currentWindowLevel[1] = vol->autoWindowLevel[1];
    vol->voxelsize.resize(3);
    vol->voxelsize[0] = 1;
    vol->voxelsize[1] = 1;
    vol->voxelsize[2] = 1;

    volumes->push_back( vol );

    return volumes;
}
