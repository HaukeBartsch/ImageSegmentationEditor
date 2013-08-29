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

/*! \file dataHandlerString.cpp
    \brief Implementation of the base class for the string handlers.

*/

#include <sstream>
#include <iomanip>

#include "../../base/include/exception.h"
#include "../include/dataHandlerString.h"


namespace puntoexe
{

namespace imebra
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataHandlerString
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse the buffer
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::parseBuffer(const ptr<memory>& memoryBuffer)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::parseBuffer");

	m_strings.clear();

	// Convert the string to unicode
	///////////////////////////////////////////////////////////
	std::wstring tempBufferUnicode;
	imbxUint32 tempBufferSize(memoryBuffer->size());
	if(tempBufferSize != 0)
	{
		tempBufferUnicode = convertToUnicode(std::string((char*)(memoryBuffer->data()), tempBufferSize));
	}

	// Remove the extra spaces and zero bytes
	///////////////////////////////////////////////////////////
	size_t removeTrail;
	for(removeTrail = tempBufferUnicode.size(); removeTrail != 0 && (tempBufferUnicode[removeTrail - 1] == 0x20 || tempBufferUnicode[removeTrail - 1] == 0x0); --removeTrail){};
	tempBufferUnicode.resize(removeTrail);

	if( getSeparator() == 0)
	{
		m_strings.push_back(tempBufferUnicode);
		return;
	}

	size_t nextPosition;
	imbxUint32 unitSize=getUnitSize();
	for(size_t firstPosition = 0; firstPosition<tempBufferUnicode.size(); firstPosition = nextPosition)
	{
		nextPosition = tempBufferUnicode.find(getSeparator(), firstPosition);
		bool bSepFound=(nextPosition != tempBufferUnicode.npos);
		if(!bSepFound)
			nextPosition = tempBufferUnicode.size();

		if(unitSize && (nextPosition-firstPosition) > (size_t)unitSize)
			nextPosition=firstPosition+unitSize;

		m_strings.push_back(tempBufferUnicode.substr(firstPosition, nextPosition-firstPosition));

		if(bSepFound)
		{
			++nextPosition;
		}
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Rebuild the buffer
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::buildBuffer(const ptr<memory>& memoryBuffer)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::buildBuffer");

	std::wstring completeString;
	for(size_t stringsIterator = 0; stringsIterator < m_strings.size(); ++stringsIterator)
	{
		if(stringsIterator)
		{
			completeString += getSeparator();
		}
		completeString += m_strings[stringsIterator];
	}

	std::string asciiString = convertFromUnicode(completeString, &m_charsetsList);

	memoryBuffer->assign((imbxUint8*)asciiString.c_str(), (imbxUint32)asciiString.size());

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns TRUE if the pointer is valid
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool dataHandlerString::pointerIsValid(const imbxUint32 index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::pointerIsValid");

	return index < m_strings.size();

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get data element as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
imbxInt32 dataHandlerString::getSignedLong(const imbxUint32 index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::getSignedLong");

	std::wstring tempString = getUnicodeString(index);
	std::wistringstream convStream(tempString);
	imbxInt32 value;
	convStream >> value;
	return value;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get data element as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
imbxUint32 dataHandlerString::getUnsignedLong(const imbxUint32 index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::getUnsignedLong");

	std::wstring tempString = getUnicodeString(index);
	std::wistringstream convStream(tempString);
	imbxUint32 value;
	convStream >> value;
	return value;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get data element as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double dataHandlerString::getDouble(const imbxUint32 index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::getDouble");

	std::wstring tempString = getUnicodeString(index);
	std::wistringstream convStream(tempString);
	double value;
	convStream >> value;
	return value;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get data element as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataHandlerString::getString(const imbxUint32 index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::getString");

        charsetsList::tCharsetsList localCharsetsList(m_charsetsList);
	return convertFromUnicode(getUnicodeString(index), &localCharsetsList);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get data element as an unicode string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataHandlerString::getUnicodeString(const imbxUint32 index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::getUnicodeString");

	if(index >= m_strings.size())
	{
		return L"";
	}
	return m_strings[index];

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set data element as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::setSignedLong(const imbxUint32 index, const imbxInt32 value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::setSignedLong");

	std::wostringstream convStream;
	convStream << value;
	setUnicodeString(index, convStream.str());

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set data element as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::setUnsignedLong(const imbxUint32 index, const imbxUint32 value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::setUnsignedLong");

	std::wostringstream convStream;
	convStream << value;
	setUnicodeString(index, convStream.str());

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set data element as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::setDouble(const imbxUint32 index, const double value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::setDouble");

	std::wostringstream convStream;
	convStream << std::fixed << value;
	setUnicodeString(index, convStream.str());

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set data element as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::setString(const imbxUint32 index, const std::string& value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::setString");

	setUnicodeString(index, convertToUnicode(value));

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set data element as an string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::setUnicodeString(const imbxUint32 index, const std::wstring& value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::setUnicodeString");

	if(index >= m_strings.size())
        {
            return;
        }
        imbxUint32 stringMaxSize(maxSize());

        if(stringMaxSize > 0 && value.size() > stringMaxSize)
        {
            m_strings[index] = value.substr(0, stringMaxSize);
            return;
        }
        m_strings[index] = value;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the buffer's size (in data elements)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerString::setSize(const imbxUint32 elementsNumber)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::setSize");

	if(getSeparator() == 0)
	{
		m_strings.resize(1, L"");
                return;
	}

	m_strings.resize(elementsNumber, L"");

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the size in strings
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
imbxUint32 dataHandlerString::getSize() const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerString::getSize");

	return (imbxUint32)m_strings.size();

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the max size of a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
imbxUint32 dataHandlerString::maxSize() const
{
	return 0;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the separator
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
wchar_t dataHandlerString::getSeparator() const
{
	return L'\\';
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Convert a string to unicode, without using the dicom
//  charsets
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataHandlerString::convertToUnicode(const std::string& value) const
{
	size_t stringLength = value.size();
	std::wstring returnString;
	returnString.resize(stringLength, 0);
	for(size_t copyString = 0; copyString < stringLength; ++copyString)
	{
		returnString[copyString]=(wchar_t)value[copyString];
	}
	return returnString;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Convert a string from unicode, without using the dicom 
//  charsets
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataHandlerString::convertFromUnicode(const std::wstring& value, charsetsList::tCharsetsList* /* pCharsetsList */) const
{
	size_t stringLength = value.size();
	std::string returnString;
	returnString.resize(stringLength);
	for(size_t copyString = 0; copyString < stringLength; ++copyString)
	{
		returnString[copyString]=(char)value[copyString];
	}
	return returnString;
}


} // namespace handlers

} // namespace imebra

} // namespace puntoexe
