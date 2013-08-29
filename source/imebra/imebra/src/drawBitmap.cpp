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

/*! \file drawBitmap.cpp
    \brief Implementation of the transform drawBitmap.

*/

#include "../include/drawBitmap.h"
#include "../include/image.h"
#include "../include/colorTransformsFactory.h"
#include "../include/transformHighBit.h"

namespace puntoexe
{

namespace imebra
{


drawBitmap::drawBitmap(ptr<image> sourceImage, ptr<transforms::transformsChain> transformsChain):
	m_image(sourceImage), m_transformsChain(new transforms::transformsChain)
{
	if(transformsChain != 0 && !transformsChain->isEmpty())
	{
		m_transformsChain->addTransform(transformsChain);
	}

	// Allocate the transforms that obtain an RGB24 image
	std::wstring initialColorSpace;
	if(m_transformsChain->isEmpty())
	{
		initialColorSpace = m_image->getColorSpace();
	}
	else
	{
		ptr<image> startImage(m_transformsChain->allocateOutputImage(m_image, 1, 1));
		initialColorSpace = startImage->getColorSpace();
	}
	transforms::colorTransforms::colorTransformsFactory* pColorTransformsFactory(transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
	ptr<transforms::colorTransforms::colorTransform> rgbColorTransform(pColorTransformsFactory->getTransform(initialColorSpace, L"RGB"));
	if(rgbColorTransform != 0)
	{
		m_transformsChain->addTransform(rgbColorTransform);
	}

	imbxUint32 width, height;
	m_image->getSize(&width, &height);
	if(m_transformsChain->isEmpty())
	{
		m_finalImage = m_image;
	}
	else
	{
		m_finalImage = m_transformsChain->allocateOutputImage(m_image, width, 1);
	}

	if(m_finalImage->getDepth() != image::depthU8 || m_finalImage->getHighBit() != 7)
	{
		m_finalImage = new image;
		m_finalImage->create(width, 1, image::depthU8, L"RGB", 7);
		m_transformsChain->addTransform(new transforms::transformHighBit);
	}
}


} // namespace imebra

} // namespace puntoexe
