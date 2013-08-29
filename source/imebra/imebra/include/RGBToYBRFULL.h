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

/*! \file RGBToYBRFULL.h
    \brief Declaration of the class RGBToYBRFULL.

*/

#if !defined(imebraRGBToYBRFULL_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
#define imebraRGBToYBRFULL_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_

#include "colorTransform.h"


///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

namespace imebra
{

namespace transforms
{

namespace colorTransforms
{

/// \addtogroup group_transforms
///
/// @{

///////////////////////////////////////////////////////////
/// \brief Transforms an image from the colorspace 
///         RGB into the color space YBR_FULL.
///
/// The input image has to have the colorspace RGB,
///  while the output image is created by the transform
///  and will have the colorspace YBR_FULL.
///
///////////////////////////////////////////////////////////
class RGBToYBRFULL: public colorTransform
{
public:
	virtual std::wstring getInitialColorSpace();
	virtual std::wstring getFinalColorSpace();
	virtual ptr<colorTransform> createColorTransform();

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
            checkColorSpaces(inputHandlerColorSpace, outputHandlerColorSpace);

            inputType* pInputMemory(inputHandlerData);
            outputType* pOutputMemory(outputHandlerData);

            pInputMemory += (inputTopLeftY * inputHandlerWidth + inputTopLeftX) * 3;
            pOutputMemory += (outputTopLeftY * outputHandlerWidth + outputTopLeftX) * 3;

            imbxInt32 outputMiddleValue(outputHandlerMinValue + outputHandlerNumValues / 2);

            imbxInt32 sourceR, sourceG, sourceB;
            if(inputHandlerNumValues == outputHandlerNumValues)
            {
                for(; inputHeight != 0; --inputHeight)
                {
                    for(int scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                    {
                        sourceR = (imbxInt32) (*(pInputMemory++)) - inputHandlerMinValue;
                        sourceG = (imbxInt32) (*(pInputMemory++)) - inputHandlerMinValue;
                        sourceB = (imbxInt32) (*(pInputMemory++)) - inputHandlerMinValue;
        				*(pOutputMemory++) = (outputType) ( (((imbxInt32)4899 * sourceR + (imbxInt32)9617 * sourceG + (imbxInt32)1868 * sourceB) >> 14) + outputHandlerMinValue );
                        *(pOutputMemory++) = (outputType) ( outputMiddleValue + (((imbxInt32)8192 * sourceB - (imbxInt32)2764 * sourceR - (imbxInt32)5428 * sourceG + (imbxInt32)8192) >> 14) );
                        *(pOutputMemory++) = (outputType) ( outputMiddleValue + (((imbxInt32)8192 * sourceR - (imbxInt32)6860 * sourceG - (imbxInt32)1332 * sourceB + (imbxInt32)8192) >> 14) );
                    }
                    pInputMemory += (inputHandlerWidth - inputWidth) * 3;
                    pOutputMemory += (outputHandlerWidth - inputWidth) * 3;
                }
            }
            else
            {
                for(; inputHeight != 0; --inputHeight)
                {
                    for(int scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                    {
                        sourceR = (imbxInt32) (*(pInputMemory++)) - inputHandlerMinValue;
                        sourceG = (imbxInt32) (*(pInputMemory++)) - inputHandlerMinValue;
                        sourceB = (imbxInt32) (*(pInputMemory++)) - inputHandlerMinValue;
        				*(pOutputMemory++) = (outputType) ( ((((imbxInt32)4899 * sourceR+(imbxInt32)9617 * sourceG+(imbxInt32)1868 * sourceB) >> 14) * outputHandlerNumValues) / inputHandlerNumValues + outputHandlerMinValue );
                        *(pOutputMemory++) = (outputType) ( outputMiddleValue + ((((imbxInt32)8192 * sourceB - (imbxInt32)2764 * sourceR - (imbxInt32)5428 * sourceG + (imbxInt32)8192) >> 14) * outputHandlerNumValues) / inputHandlerNumValues );
                        *(pOutputMemory++) = (outputType) ( outputMiddleValue + ((((imbxInt32)8192 * sourceR - (imbxInt32)6860 * sourceG - (imbxInt32)1332 * sourceB + (imbxInt32)8192) >> 14) * outputHandlerNumValues) / inputHandlerNumValues );
                    }
                    pInputMemory += (inputHandlerWidth - inputWidth) * 3;
                    pOutputMemory += (outputHandlerWidth - inputWidth) * 3;
                }
            }
        }
};

/// @}

} // namespace colorTransforms

} // namespace transforms

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraRGBToYBRFULL_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
