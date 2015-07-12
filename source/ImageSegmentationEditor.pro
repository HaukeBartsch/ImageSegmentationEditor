QT += widgets
qtHaveModule(printsupport): QT += printsupport
QT += network
QT += opengl
QT += webkit webkitwidgets
unix:LIBS += -lpthread -liconv -lz
windows:DEFINES += _WIN32_WINNT=0x0501 WINVER=0x0501 NTDDI_VERSION=0x05010000

HEADERS       = \
    mainwindow.h \
    readmgz.h \
    volume.h \
    Types.h \
    EventProcessing.h \
    undoredo.h \
    preferences.h \
    exportsnapshots.h \
    setbrightnesscontrast.h \
    readdicom.h \
    imebra/include/thread.h \
    imebra/include/streamWriter.h \
    imebra/include/streamReader.h \
    imebra/include/streamController.h \
    imebra/include/stream.h \
    imebra/include/nullStream.h \
    imebra/include/memoryStream.h \
    imebra/include/memory.h \
    imebra/include/huffmanTable.h \
    imebra/include/exception.h \
    imebra/include/criticalSection.h \
    imebra/include/configuration.h \
    imebra/include/charsetConversionWindows.h \
    imebra/include/charsetConversionICU.h \
    imebra/include/charsetConversionIconv.h \
    imebra/include/charsetConversion.h \
    imebra/include/baseStream.h \
    imebra/imebra/include/YBRPARTIALToRGB.h \
    imebra/imebra/include/YBRFULLToRGB.h \
    imebra/imebra/include/YBRFULLToMONOCHROME2.h \
    imebra/imebra/include/waveform.h \
    imebra/imebra/include/VOILUT.h \
    imebra/imebra/include/viewHelper.h \
    imebra/imebra/include/transformsChain.h \
    imebra/imebra/include/transformHighBit.h \
    imebra/imebra/include/transform.h \
    imebra/imebra/include/transaction.h \
    imebra/base/include/thread.h \
    imebra/base/include/streamWriter.h \
    imebra/base/include/streamReader.h \
    imebra/base/include/streamController.h \
    imebra/base/include/stream.h \
    imebra/imebra/include/RGBToYBRPARTIAL.h \
    imebra/imebra/include/RGBToYBRFULL.h \
    imebra/imebra/include/RGBToMONOCHROME2.h \
    imebra/imebra/include/PALETTECOLORToRGB.h \
    imebra/base/include/nullStream.h \
    imebra/imebra/include/MONOCHROME2ToYBRFULL.h \
    imebra/imebra/include/MONOCHROME2ToRGB.h \
    imebra/imebra/include/MONOCHROME1ToRGB.h \
    imebra/imebra/include/MONOCHROME1ToMONOCHROME2.h \
    imebra/imebra/include/modalityVOILUT.h \
    imebra/base/include/memoryStream.h \
    imebra/base/include/memory.h \
    imebra/imebra/include/LUT.h \
    imebra/imebra/include/jpegCodec.h \
    imebra/imebra/include/imebraDoc.h \
    imebra/imebra/include/imebra.h \
    imebra/imebra/include/image.h \
    imebra/base/include/huffmanTable.h \
    imebra/base/include/exception.h \
    imebra/imebra/include/drawBitmap.h \
    imebra/imebra/include/dicomDir.h \
    imebra/imebra/include/dicomDict.h \
    imebra/imebra/include/dicomCodec.h \
    imebra/imebra/include/dataSet.h \
    imebra/imebra/include/dataHandlerTime.h \
    imebra/imebra/include/dataHandlerStringUT.h \
    imebra/imebra/include/dataHandlerStringUnicode.h \
    imebra/imebra/include/dataHandlerStringUI.h \
    imebra/imebra/include/dataHandlerStringST.h \
    imebra/imebra/include/dataHandlerStringSH.h \
    imebra/imebra/include/dataHandlerStringPN.h \
    imebra/imebra/include/dataHandlerStringLT.h \
    imebra/imebra/include/dataHandlerStringLO.h \
    imebra/imebra/include/dataHandlerStringIS.h \
    imebra/imebra/include/dataHandlerStringDS.h \
    imebra/imebra/include/dataHandlerStringCS.h \
    imebra/imebra/include/dataHandlerStringAS.h \
    imebra/imebra/include/dataHandlerStringAE.h \
    imebra/imebra/include/dataHandlerString.h \
    imebra/imebra/include/dataHandlerNumeric.h \
    imebra/imebra/include/dataHandlerDateTimeBase.h \
    imebra/imebra/include/dataHandlerDateTime.h \
    imebra/imebra/include/dataHandlerDate.h \
    imebra/imebra/include/dataHandler.h \
    imebra/imebra/include/dataGroup.h \
    imebra/imebra/include/dataCollection.h \
    imebra/imebra/include/data.h \
    imebra/base/include/criticalSection.h \
    imebra/base/include/configuration.h \
    imebra/imebra/include/colorTransformsFactory.h \
    imebra/imebra/include/colorTransform.h \
    imebra/imebra/include/codecFactory.h \
    imebra/imebra/include/codec.h \
    imebra/imebra/include/charsetsList.h \
    imebra/base/include/charsetConversionWindows.h \
    imebra/base/include/charsetConversionICU.h \
    imebra/base/include/charsetConversionIconv.h \
    imebra/base/include/charsetConversion.h \
    imebra/imebra/include/bufferStream.h \
    imebra/imebra/include/buffer.h \
    imebra/base/include/baseStream.h \
    imebra/base/include/baseObject.h \
    helpdialog.h \
    statistics.h
SOURCES       = \
                main.cpp \
    mainwindow.cpp \
    readmgz.cpp \
    volume.cpp \
    undoredo.cpp \
    preferences.cpp \
    exportsnapshots.cpp \
    setbrightnesscontrast.cpp \
    readdicom.cpp \
    imebra/imebra/src/YBRPARTIALToRGB.cpp \
    imebra/imebra/src/YBRFULLToRGB.cpp \
    imebra/imebra/src/YBRFULLToMONOCHROME2.cpp \
    imebra/imebra/src/waveform.cpp \
    imebra/imebra/src/VOILUT.cpp \
    imebra/imebra/src/viewHelper.cpp \
    imebra/imebra/src/transformsChain.cpp \
    imebra/imebra/src/transformHighBit.cpp \
    imebra/imebra/src/transform.cpp \
    imebra/imebra/src/transaction.cpp \
    imebra/base/src/thread.cpp \
    imebra/base/src/streamWriter.cpp \
    imebra/base/src/streamReader.cpp \
    imebra/base/src/streamController.cpp \
    imebra/base/src/stream.cpp \
    imebra/imebra/src/RGBToYBRPARTIAL.cpp \
    imebra/imebra/src/RGBToYBRFULL.cpp \
    imebra/imebra/src/RGBToMONOCHROME2.cpp \
    imebra/imebra/src/PALETTECOLORToRGB.cpp \
    imebra/imebra/src/MONOCHROME2ToYBRFULL.cpp \
    imebra/imebra/src/MONOCHROME2ToRGB.cpp \
    imebra/imebra/src/MONOCHROME1ToRGB.cpp \
    imebra/imebra/src/MONOCHROME1ToMONOCHROME2.cpp \
    imebra/imebra/src/modalityVOILUT.cpp \
    imebra/base/src/memoryStream.cpp \
    imebra/base/src/memory.cpp \
    imebra/imebra/src/LUT.cpp \
    imebra/imebra/src/jpegCodec.cpp \
    imebra/imebra/src/image.cpp \
    imebra/base/src/huffmanTable.cpp \
    imebra/base/src/exception.cpp \
    imebra/imebra/src/drawBitmap.cpp \
    imebra/imebra/src/dicomDir.cpp \
    imebra/imebra/src/dicomDict.cpp \
    imebra/imebra/src/dicomCodec.cpp \
    imebra/imebra/src/dataSet.cpp \
    imebra/imebra/src/dataHandlerTime.cpp \
    imebra/imebra/src/dataHandlerStringUT.cpp \
    imebra/imebra/src/dataHandlerStringUnicode.cpp \
    imebra/imebra/src/dataHandlerStringUI.cpp \
    imebra/imebra/src/dataHandlerStringST.cpp \
    imebra/imebra/src/dataHandlerStringSH.cpp \
    imebra/imebra/src/dataHandlerStringPN.cpp \
    imebra/imebra/src/dataHandlerStringLT.cpp \
    imebra/imebra/src/dataHandlerStringLO.cpp \
    imebra/imebra/src/dataHandlerStringIS.cpp \
    imebra/imebra/src/dataHandlerStringDS.cpp \
    imebra/imebra/src/dataHandlerStringCS.cpp \
    imebra/imebra/src/dataHandlerStringAS.cpp \
    imebra/imebra/src/dataHandlerStringAE.cpp \
    imebra/imebra/src/dataHandlerString.cpp \
    imebra/imebra/src/dataHandlerDateTimeBase.cpp \
    imebra/imebra/src/dataHandlerDateTime.cpp \
    imebra/imebra/src/dataHandlerDate.cpp \
    imebra/imebra/src/dataHandler.cpp \
    imebra/imebra/src/dataGroup.cpp \
    imebra/imebra/src/data.cpp \
    imebra/base/src/criticalSection.cpp \
    imebra/imebra/src/colorTransformsFactory.cpp \
    imebra/imebra/src/colorTransform.cpp \
    imebra/imebra/src/codecFactory.cpp \
    imebra/imebra/src/codec.cpp \
    imebra/imebra/src/charsetsList.cpp \
    imebra/base/src/charsetConversionWindows.cpp \
    imebra/base/src/charsetConversionICU.cpp \
    imebra/base/src/charsetConversionIconv.cpp \
    imebra/base/src/charsetConversion.cpp \
    imebra/imebra/src/buffer.cpp \
    imebra/base/src/baseStream.cpp \
    imebra/base/src/baseObject.cpp \
    helpdialog.cpp \
    statistics.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/imageviewer
INSTALLS += target


wince*: {
   DEPLOYMENT_PLUGIN += qjpeg qgif
}

simulator: warning(This example might not fully work on Simulator platform)

FORMS += \
    mainwindow.ui \
    preferences.ui \
    exportsnapshots.ui \
    setbrightnesscontrast.ui \
    helpdialog.ui \
    statistics.ui

OTHER_FILES += \
    Help/index.html
