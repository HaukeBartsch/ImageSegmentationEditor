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

/*! \file transformHighBit.h
    \brief Declaration of the class transformHighBit.

*/

#if !defined(imebraTransformHighBit_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
#define imebraTransformHighBit_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_

#include "transform.h"
#include "colorTransformsFactory.h"

namespace puntoexe
{

namespace imebra
{

namespace transforms
{

/// \addtogroup group_transforms
///
/// @{

class transformHighBitException: public transformException
{
public:
		transformHighBitException(const std::string& message): transformException(message){}
};

class transformHighBitDifferentColorSpaces: public transformHighBitException
{
public:
    transformHighBitDifferentColorSpaces(const std::string& message): transformHighBitException(message){}
};


///////////////////////////////////////////////////////////
/// \brief Shift the image's content to adapt it to the
///         specified high bit.
///
/// In order to use this transform, both the input and
///  the output image must be defined by calling
///  declareInputImage() and declareOutputImage().
///
/// The input image is copied into the output image but
///  all the values are shifted to be fitted in the
///  output image's high bit settings.
///
///////////////////////////////////////////////////////////
class transformHighBit: public transformHandlers
{
public:
        virtual ptr<image> allocateOutputImage(ptr<image> pInputImage, imbxUint32 width, imbxUint32 height);

        DEFINE_RUN_TEMPLATE_TRANSFORM;

        template <class inputType, class outputType>
        void templateTransform(
            inputType* inputHandlerData, size_t /* inputHandlerSize */, imbxUint32 inputHandlerWidth, const std::wstring& inputHandlerColorSpace,
            ptr<palette> /* inputPalette */,
            imbxInt32 inputHandlerMinValue, imbxUint32 inputHandlerNumValues,
            imbxInt32 inputTopLeftX, imbxInt32 inputTopLeftY, imbxInt32 inputWidth, imbxInt32 inputHeight,
            outputType* outputHandlerData, size_t /* outputHandlerSize */, imbxInt32 outputHandlerWidth, const std::wstring& outputHandlerColorSpace,
            ptr<palette> /* outputPalette */,
            imbxInt32 outputHandlerMinValue, imbxUint32 outputHandlerNumValues,
            imbxInt32 outputTopLeftX, imbxInt32 outputTopLeftY)

        {
            if(colorTransforms::colorTransformsFactory::normalizeColorSpace(inputHandlerColorSpace) !=
               colorTransforms::colorTransformsFactory::normalizeColorSpace(outputHandlerColorSpace))
            {
                throw transformHighBitDifferentColorSpaces("The input and output image must have the same color space");
            }

            imbxInt32 numChannels(colorTransforms::colorTransformsFactory::getNumberOfChannels(inputHandlerColorSpace));

            inputType* pInputMemory(inputHandlerData);
            outputType* pOutputMemory(outputHandlerData);

            pInputMemory += (inputTopLeftY * inputHandlerWidth + inputTopLeftX) * numChannels;
            pOutputMemory += (outputTopLeftY * outputHandlerWidth + outputTopLeftX) * numChannels;

            if(inputHandlerNumValues == outputHandlerNumValues)
            {
                imbxInt32 offset(inputHandlerMinValue - outputHandlerMinValue);
                for(; inputHeight != 0; --inputHeight)
                {
                    for(int scanPixels(inputWidth * numChannels); scanPixels != 0; --scanPixels)
                    {
                        *(pOutputMemory++) = (outputType)((imbxInt32)*(pInputMemory++) - offset);
                    }
                    pInputMemory += (inputHandlerWidth - inputWidth) * numChannels;
                    pOutputMemory += (outputHandlerWidth - inputWidth) * numChannels;
                }
            }
            else
            {
                for(; inputHeight != 0; --inputHeight)
                {
                    for(int scanPixels(inputWidth * numChannels); scanPixels != 0; --scanPixels)
                    {
                        *pOutputMemory++ = (outputType)((((imbxInt32)*(pInputMemory++) - inputHandlerMinValue) * outputHandlerNumValues) / inputHandlerNumValues + outputHandlerMinValue);
                    }
                    pInputMemory += (inputHandlerWidth - inputWidth) * numChannels;
                    pOutputMemory += (outputHandlerWidth - inputWidth) * numChannels;
                }
            }
        }

};

/// @}

} // namespace transforms

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraTransformHighBit_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
