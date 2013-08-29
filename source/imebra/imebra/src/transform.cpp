/*

Imebra 2011 build 2013-07-16_08-42-08

Imebra: a C++ Dicom library

Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Paolo Brandoli/Binarno s.p.
All rights reserved.

This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2 as published by
 the Free Software Foundation.

This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

-------------------

If you want to use Imebra commercially then you have to buy the commercial
 support available at http://imebra.com

After you buy the commercial support then you can use Imebra according
 to the terms described in the Imebra Commercial License Version 1.
A copy of the Imebra Commercial License Version 1 is available in the
 documentation pages.

Imebra is available at http://imebra.com

The author can be contacted by email at info@binarno.com or by mail at
 the following address:
 Paolo Brandoli
 Rakuseva 14
 1000 Ljubljana
 Slovenia



*/

/*! \file transform.cpp
    \brief Implementation of the base class used by the transforms.

*/

#include "../../base/include/exception.h"
#include "../include/transform.h"
#include "../include/image.h"
#include "../include/transformHighBit.h"


namespace puntoexe
{

namespace imebra
{

namespace transforms
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Declare an input parameter
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool transform::isEmpty()
{
	return false;
}


void transformHandlers::runTransform(
            const ptr<image>& inputImage,
            imbxUint32 inputTopLeftX, imbxUint32 inputTopLeftY, imbxUint32 inputWidth, imbxUint32 inputHeight,
            const ptr<image>& outputImage,
            imbxUint32 outputTopLeftX, imbxUint32 outputTopLeftY)
{
    PUNTOEXE_FUNCTION_START(L"transformHandlers::runTransform");

    imbxUint32 inputImageWidth, inputImageHeight;
    inputImage->getSize(&inputImageWidth, &inputImageHeight);
    imbxUint32 outputImageWidth, outputImageHeight;
    outputImage->getSize(&outputImageWidth, &outputImageHeight);

    if(inputTopLeftX + inputWidth > inputImageWidth ||
        inputTopLeftY + inputHeight > inputImageHeight ||
        outputTopLeftX + inputWidth > outputImageWidth ||
        outputTopLeftY + inputHeight > outputImageHeight)
    {
        PUNTOEXE_THROW(transformExceptionInvalidArea, "The input and/or output areas are invalid");
    }

    imbxUint32 rowSize, numPixels, channels;
	ptr<handlers::dataHandlerNumericBase> inputHandler(inputImage->getDataHandler(false, &rowSize, &numPixels, &channels));
	ptr<palette> inputPalette(inputImage->getPalette());
	std::wstring inputColorSpace(inputImage->getColorSpace());
	imbxUint32 inputHighBit(inputImage->getHighBit());
	imbxUint32 inputNumValues((imbxUint32)1 << (inputHighBit + 1));
	imbxInt32 inputMinValue(0);
	image::bitDepth inputDepth(inputImage->getDepth());
	if(inputDepth == image::depthS16 || inputDepth == image::depthS8)
	{
		inputMinValue -= (imbxInt32)(inputNumValues >> 1);
	}

	ptr<handlers::dataHandlerNumericBase> outputHandler(outputImage->getDataHandler(false, &rowSize, &numPixels, &channels));
	ptr<palette> outputPalette(outputImage->getPalette());
	std::wstring outputColorSpace(outputImage->getColorSpace());
	imbxUint32 outputHighBit(outputImage->getHighBit());
	imbxUint32 outputNumValues((imbxUint32)1 << (outputHighBit + 1));
	imbxInt32 outputMinValue(0);
	image::bitDepth outputDepth(outputImage->getDepth());
	if(outputDepth == image::depthS16 || outputDepth == image::depthS8)
	{
		outputMinValue -= (imbxInt32)(outputNumValues >> 1);
	}

	if(isEmpty())
	{
		ptr<transformHighBit> emptyTransform(new transformHighBit);
		emptyTransform->runTransformHandlers(inputHandler, inputImageWidth, inputColorSpace, inputPalette, inputMinValue, inputNumValues,
											 inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,
											 outputHandler, outputImageWidth, outputColorSpace, outputPalette, outputMinValue, outputNumValues,
											 outputTopLeftX, outputTopLeftY);
		return;
	}

	runTransformHandlers(inputHandler, inputImageWidth, inputColorSpace, inputPalette, inputMinValue, inputNumValues,
		inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,
		outputHandler, outputImageWidth, outputColorSpace, outputPalette, outputMinValue, outputNumValues,
		outputTopLeftX, outputTopLeftY);

    PUNTOEXE_FUNCTION_END();
}


} // namespace transforms

} // namespace imebra

} // namespace puntoexe
