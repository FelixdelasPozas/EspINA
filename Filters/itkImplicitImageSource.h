/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#ifndef ITKIMPLICITIMAGESOURCE_H
#define ITKIMPLICITIMAGESOURCE_H

#include "itkImageSource.h"

#include <vtkSmartPointer.h>
#include <vtkImplicitFunction.h>

#include <QList>

namespace itk
{

template< typename TOutputImage >
class ITK_EXPORT ImplicitImageSource
: public ImageSource< TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef ImplicitImageSource           Self;
  typedef ImageSource< TOutputImage > Superclass;
  typedef SmartPointer< Self >        Pointer;
  typedef SmartPointer< const Self >  ConstPointer;

  /** Typedef for the output image PixelType. */
  typedef typename TOutputImage::PixelType OutputImagePixelType;

  /** Typedef to describe the output image region type. */
  typedef typename TOutputImage::RegionType OutputImageRegionType;

  /** Typedef to specify a list of implicit functions to define the image */
  typedef QList<vtkImplicitFunction *> FunctionList;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImplicitImageSource, ImageSource);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Basic types from the OutputImageType */
  typedef typename TOutputImage::SizeType         SizeType;
  typedef typename TOutputImage::IndexType        IndexType;
  typedef typename TOutputImage::SpacingType      SpacingType;
  typedef typename TOutputImage::PointType        PointType;
  typedef typename SizeType::SizeValueType        SizeValueType;
  typedef SizeValueType                           SizeValueArrayType[TOutputImage::ImageDimension];
  typedef typename TOutputImage::SpacingValueType SpacingValueType;
  typedef SpacingValueType                        SpacingValueArrayType[TOutputImage::ImageDimension];
  typedef typename TOutputImage::PointValueType   PointValueType;
  typedef PointValueType                          PointValueArrayType[TOutputImage::ImageDimension];

  /** Set/Get size of the output image */
  itkSetMacro(Size, SizeType);
  virtual void SetSize(SizeValueArrayType sizeArray);
  virtual const SizeValueType * GetSize() const;

  /** Set/Get spacing of the output image */
  itkSetMacro(Spacing, SpacingType);
  virtual void SetSpacing(SpacingValueArrayType spacingArray);
  virtual const SpacingValueType * GetSpacing() const;

  /** Set/Get origin of the output image */
  itkSetMacro(Origin, PointType);
  virtual void SetOrigin(PointValueArrayType originArray);
  virtual const PointValueType * GetOrigin() const;

  /** Set implicit functions to create the image with */
  virtual void SetImplicitFunctions(FunctionList functions);

protected:
  ImplicitImageSource();
  ~ImplicitImageSource();
  void PrintSelf(std::ostream & os, Indent indent) const;

  virtual void
  ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread,
                       ThreadIdType threadId);

  virtual void GenerateOutputInformation();

private:
  ImplicitImageSource(const ImplicitImageSource &); //purposely not implemented
  void operator=(const ImplicitImageSource &);    //purposely not implemented

  SizeType    m_Size;       //size of the output image
  SpacingType m_Spacing;    //spacing
  PointType   m_Origin;     //origin

  // The following variables are deprecated, and provided here just for
  // backward compatibility. It use is discouraged.
  mutable PointValueArrayType   m_OriginArray;
  mutable SpacingValueArrayType m_SpacingArray;

  FunctionList m_functions;
};
} // end namespace itk

#include "itkImplicitImageSource.hxx"

#endif // ITKIMPLICITIMAGESOURCE_H