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

/*! \file stream.h
    \brief Declaration of the stream class.

*/

#if !defined(imebraStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include "baseStream.h"

#include <ios>
#include <stdio.h>

///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

/// \addtogroup group_baseclasses
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class derives from the baseStream 
///         class and implements a file stream.
///
/// This class can be used to read/write on physical files
///  in the mass storage.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class stream : public baseStream
{
protected:
	// Destructor
	///////////////////////////////////////////////////////////
	virtual ~stream();

public:
	// Constructor
	///////////////////////////////////////////////////////////
	stream(): m_openFile(0){}

	/// \brief Open a file.
	///
	/// The function uses the standard library function
	///  fopen to open the specified file.
	///
	/// The created file object will be automatically
	///  closed and destroyed when one of the following events
	///  will occur:
	/// - the stream class will be destroyed
	/// - this function will be called again
	///
	/// @param fileName the name of the file to be opened
	/// @param mode     the opening mode.
	///                 Can be the combination of one or
	///                 more of the following values:
	///                 - ios_base::in the file will be opened
	///                   for reading operations
	///                 - ios_base::out the file will be
	///                   opened for writing operations
	///                 - ios_base::app the writing operations
	///                   will append data to the existing file
	///                 - ios_base::trunc the existing file
	///                   will be truncated to zero length
	///                 - ios_base::binary the file will be
	///                   opened in binary mode. Please note
	///                   that this flag is useless, since all
	///                   the files ARE OPENED IN BINARY MODE.
	///
	///////////////////////////////////////////////////////////
	void openFile(const std::string& fileName, const int mode);

	void openFile(const std::wstring& fileName, const int mode);

	///////////////////////////////////////////////////////////
	//
	// Virtual stream's functions
	//
	///////////////////////////////////////////////////////////
	virtual void write(imbxUint32 startPosition, const imbxUint8* pBuffer, imbxUint32 bufferLength);
	virtual imbxUint32 read(imbxUint32 startPosition, imbxUint8* pBuffer, imbxUint32 bufferLength);

protected:
	FILE* m_openFile;
};

///@}

} // namespace puntoexe


#endif // !defined(imebraStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
