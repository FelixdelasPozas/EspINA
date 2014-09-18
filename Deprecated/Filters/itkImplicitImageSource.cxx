/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ITKIMPLICITIMAGESOURCE_HXX
#define ITKIMPLICITIMAGESOURCE_HXX

// ESPINA
#include "itkImplicitImageSource.h"
#include <Core/EspinaTypes.h>

// ITK
#include <itkImageRegionIterator.h>
#include <itkObjectFactory.h>
#include <itkProgressReporter.h>
#include <itkImageRegionIteratorWithIndex.h>

namespace itk
{
/**
 *
 */
template< class TOutputImage >
ImplicitImageSource< TOutputImage >
::ImplicitImageSource()
{
  //Initial image is 64 wide in each direction.
  for ( unsigned int i = 0; i < TOutputImage::GetImageDimension(); i++ )
    {
    m_Size[i] = 64;
    m_Spacing[i] = 1.0;
    m_Origin[i] = 0.0;
    }
}

template< class TOutputImage >
ImplicitImageSource< TOutputImage >
::~ImplicitImageSource()
{}

template< class TOutputImage >
void
ImplicitImageSource< TOutputImage >
::SetSize(SizeValueArrayType sizeArray)
{
  const unsigned int count = TOutputImage::ImageDimension;
  unsigned int       i;

  for ( i = 0; i < count; i++ )
    {
    if ( sizeArray[i] != this->m_Size[i] )
      {
      break;
      }
    }
  if ( i < count )
    {
    this->Modified();
    for ( i = 0; i < count; i++ )
      {
      this->m_Size[i] = sizeArray[i];
      }
    }
}

template< class TOutputImage >
const typename ImplicitImageSource< TOutputImage >::SizeValueType *
ImplicitImageSource< TOutputImage >
::GetSize() const
{
  return this->m_Size.GetSize();
}

template< class TOutputImage >
void
ImplicitImageSource< TOutputImage >
::SetSpacing(SpacingValueArrayType spacingArray)
{
  const unsigned int count = TOutputImage::ImageDimension;
  unsigned int       i;

  for ( i = 0; i < count; i++ )
    {
    if ( spacingArray[i] != this->m_Spacing[i] )
      {
      break;
      }
    }
  if ( i < count )
    {
    this->Modified();
    for ( i = 0; i < count; i++ )
      {
      this->m_Spacing[i] = spacingArray[i];
      }
    }
}

template< class TOutputImage >
void
ImplicitImageSource< TOutputImage >
::SetOrigin(PointValueArrayType originArray)
{
  const unsigned int count = TOutputImage::ImageDimension;
  unsigned int       i;

  for ( i = 0; i < count; i++ )
    {
    if ( originArray[i] != this->m_Origin[i] )
      {
      break;
      }
    }
  if ( i < count )
    {
    this->Modified();
    for ( i = 0; i < count; i++ )
      {
      this->m_Origin[i] = originArray[i];
      }
    }
}

template< class TOutputImage >
const typename ImplicitImageSource< TOutputImage >::PointValueType *
ImplicitImageSource< TOutputImage >
::GetOrigin() const
{
  for ( unsigned int i = 0; i < TOutputImage::ImageDimension; i++ )
    {
    this->m_OriginArray[i] = this->m_Origin[i];
    }
  return this->m_OriginArray;
}

template< class TOutputImage >
const typename ImplicitImageSource< TOutputImage >::SpacingValueType *
ImplicitImageSource< TOutputImage >
::GetSpacing() const
{
  for ( unsigned int i = 0; i < TOutputImage::ImageDimension; i++ )
    {
    this->m_SpacingArray[i] = this->m_Spacing[i];
    }
  return this->m_SpacingArray;
}

/**
 *
 */
template< class TOutputImage >
void
ImplicitImageSource< TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  unsigned int i;
  os << indent << "Origin: [";
  for ( i = 0; i < TOutputImage::ImageDimension - 1; i++ )
    {
    os << m_Origin[i] << ", ";
    }
  os << m_Origin[i] << "]" << std::endl;

  os << indent << "Spacing: [";
  for ( i = 0; i < TOutputImage::ImageDimension - 1; i++ )
    {
    os << m_Spacing[i] << ", ";
    }
  os << m_Spacing[i] << "]" << std::endl;

  os << indent << "Size: [";
  for ( i = 0; i < TOutputImage::ImageDimension - 1; i++ )
    {
    os << m_Size[i] << ", ";
    }
  os << m_Size[i] << "]" << std::endl;
}

//----------------------------------------------------------------------------
template< typename TOutputImage >
void
ImplicitImageSource< TOutputImage >
::GenerateOutputInformation()
{
  TOutputImage *output;
  IndexType     index;

  index.Fill(0);

  output = this->GetOutput(0);

  typename TOutputImage::RegionType largestPossibleRegion;
  largestPossibleRegion.SetSize(this->m_Size);
  largestPossibleRegion.SetIndex(index);
  output->SetLargestPossibleRegion(largestPossibleRegion);

  output->SetSpacing(m_Spacing);
  output->SetOrigin(m_Origin);
}

//----------------------------------------------------------------------------
template< typename TOutputImage >
void
ImplicitImageSource< TOutputImage >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       ThreadIdType threadId)
{
  itkDebugMacro(<< "Generating a Implicit image of scalars");

  // Support progress methods/callbacks
//  ProgressReporter progress( this, threadId, outputRegionForThread.GetNumberOfPixels() );

  typename TOutputImage::Pointer image = this->GetOutput(0);

  ImageRegionIteratorWithIndex< TOutputImage > it(image, outputRegionForThread);

  for (; !it.IsAtEnd(); ++it )
  {
    double tx = it.GetIndex()[0]*m_Spacing[0];
    double ty = it.GetIndex()[1]*m_Spacing[1];
    double tz = it.GetIndex()[2]*m_Spacing[2];

    typename TOutputImage::PixelType value = 0;
    for (int i=0; i < m_functions.size(); i++)
    {
      if (m_functions.value(i)->FunctionValue(tx, ty, tz) <= 0)
      {
        value = 255;
        continue;
      }
    }
    it.Set(value);
 //   progress.CompletedPixel();
  }
}

//----------------------------------------------------------------------------
template< typename TOutputImage >
void
ImplicitImageSource< TOutputImage >
::SetImplicitFunctions(FunctionList functions)
{
  itkDebugMacro(<< "Updating Implicit Function definition");
  m_functions = functions;
}

} // end namespace itk

#endif // ITKIMPLICITIMAGESOURCE_HXX
