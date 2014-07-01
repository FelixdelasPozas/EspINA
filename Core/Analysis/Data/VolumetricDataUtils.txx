/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Core/Utils/Spatial.h>
#include <Core/Utils/Bounds.h>
#include <Core/Utils/VolumeBounds.h>
#include <itkImageRegionConstIterator.h>

namespace EspINA {

  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::RegionType equivalentRegion(const T* image, const Bounds& bounds)
  {
    typename T::SpacingType s = image->GetSpacing();
    typename T::PointType o = image->GetOrigin();

    typename T::PointType p0, p1;
    for (int i = 0; i < 3; ++i)
    {
      Axis dir = toAxis(i);

      p0[i] = bounds[2 * i];
      p1[i] = bounds[2 * i + 1];

      if (areEqual(p0[i], p1[i]) && !bounds.areUpperIncluded(dir) && !bounds.areLowerIncluded(dir))
      {
        throw Invalid_Bounds_Exception();
      }

      if (isAligned(p0[i], o[i], s[i]))
      {
        p0[i] += s[i] / 2.0;
      }

      if (isAligned(p1[i], o[i], s[i]))
      {
        if (bounds.areUpperIncluded(dir))
        {
          p1[i] += s[i] / 2.0;
        }
        else
        {
          p1[i] -= s[i] / 2.0;
        }
      }
    }

    typename T::IndexType i0, i1;
    image->TransformPhysicalPointToIndex(p0, i0);
    image->TransformPhysicalPointToIndex(p1, i1);

    typename T::RegionType region;
    region.SetIndex(i0);
    region.SetUpperIndex(i1);

    for (auto i: {0,1,2})
      if (region.GetSize(i) == 0)
        region.SetSize(i,1);

      return region;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::RegionType equivalentRegion(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds)
  {
    typename T::Pointer image = define_itkImage<T>(origin, spacing);

    return equivalentRegion<T>(image, bounds);
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds equivalentBounds(const typename T::Pointer image, const typename T::RegionType& region)
  {
    Bounds bounds;

    typename T::PointType p0, p1;

    image->TransformIndexToPhysicalPoint(region.GetIndex(), p0);
    image->TransformIndexToPhysicalPoint(region.GetUpperIndex(), p1);

    typename T::SpacingType s = image->GetSpacing();

    for (int i = 0; i < 3; ++i)
    {
      bounds[2*i]   = p0[i] - s[i]/2;
      bounds[2*i+1] = p1[i] + s[i]/2;
    }

    return bounds;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds equivalentBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region)
  {
    typename T::Pointer image = define_itkImage<T>(origin, spacing);

    return equivalentBounds<T>(image, region);
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region)
  {
    return volumeBounds<T>(define_itkImage<T>(origin, spacing), region);

    //typename T::Pointer image = define_itkImage<T>(origin, spacing);
    // return volumeBounds<T>(image, equivalentBounds<T>(image, region));
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const Bounds& bounds)
  {
    NmVector3 origin;
    for (int i = 0; i < 3; ++i) origin[i] = image->GetOrigin()[i];

    NmVector3 spacing;
    for (int i = 0; i < 3; ++i) spacing[i] = image->GetSpacing()[i];

    return volumeBounds<T>(origin, spacing, bounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const typename T::RegionType& region)
  {
    return volumeBounds<T>(image, equivalentBounds<T>(image, region));
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds)
  {
    return VolumeBounds(bounds, spacing, origin);
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds leftSliceBounds(const  T &volume)
  {
    auto slice   = volume->bounds();
    auto spacing = volume->spacing();

    slice[1] = slice[0] + spacing[0]/2.0;

    return slice;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds rightSliceBounds(const  T &volume)
  {
    auto slice   = volume->bounds();
    auto spacing = volume->spacing();

    slice[0] = slice[1] - spacing[0]/2.0;

    return slice;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds topSliceBounds(const  T &volume)
  {
    auto slice   = volume->bounds();
    auto spacing = volume->spacing();

    slice[3] = slice[2] + spacing[1]/2.0;

    return slice;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds bottomSliceBounds(const  T &volume)
  {
    auto slice   = volume->bounds();
    auto spacing = volume->spacing();

    slice[2] = slice[3] - spacing[1]/2.0;

    return slice;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds frontSliceBounds(const  T &volume)
  {
    auto slice   = volume->bounds();
    auto spacing = volume->spacing();

    slice[5] = slice[4] + spacing[2]/2.0;

    return slice;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds backSliceBounds(const  T &volume)
  {
    auto slice   = volume->bounds();
    auto spacing = volume->spacing();

    slice[4] = slice[5] - spacing[2]/2.0;

    return slice;
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  unsigned long voxelCount(const typename T::Pointer image, const typename T::ValueType value)
  {
    unsigned long count = 0;

    itk::ImageRegionConstIterator<T> it(image, image->GetLargestPossibleRegion());

    it.GoToBegin();
    while (!it.IsAtEnd())
    {
      if (it.Get()) ++count;
      ++it;
    }

    return count;
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds minimalBounds(const typename T::Pointer image, const typename T::ValueType value)
  {
    Bounds bounds;

    itk::ImageRegionConstIterator<T> it(image, image->GetLargestPossibleRegion());
    auto origin  = image->GetOrigin();
    auto spacing = image->GetSpacing();

    it.GoToBegin();
    while (!it.IsAtEnd())
    {
      if (it.Get() != value)
      {
        auto index   = it.GetIndex();
        Bounds voxelBounds;
        for (int i = 0; i < 3; ++i)
        {
          voxelBounds[2*i]   = ( index[i]    * spacing[i]) - origin[i] - spacing[i]/2;
          voxelBounds[2*i+1] = ((index[i]+1) * spacing[i]) - origin[i] - spacing[i]/2;
        }

        if (!bounds.areValid())
          bounds = voxelBounds;
        else
          bounds = boundingBox(bounds, voxelBounds);
      }
      ++it;
    }

    return bounds;
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::SpacingType ItkSpacing(const NmVector3& spacing)
  {
    typename T::SpacingType itkSpacing;

    for(int i = 0; i < 3; ++i)
      itkSpacing[i] = spacing[i];

    return itkSpacing;
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  NmVector3 ToNmVector3(typename T::SpacingType itkSpacing)
  {
    NmVector3 vector;

    for(int i = 0; i < 3; ++i)
      vector[i] = itkSpacing[i];

    return vector;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  NmVector3 ToNmVector3(typename T::PointType itkPoint)
  {
    NmVector3 vector;

    for(int i = 0; i < 3; ++i)
      vector[i] = itkPoint[i];

    return vector;
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(const typename T::Pointer volume, const Bounds &inputBounds)
  {
    using itk2vtkImageFilter = itk::ImageToVTKImageFilter<T>;
    using ExtractFilter = itk::ExtractImageFilter<T,T>;

    typename T::Pointer itkImage;

    auto spacing = volume->GetSpacing();
    auto origin = volume->GetOrigin();
    VolumeBounds iBounds(inputBounds, NmVector3{spacing[0], spacing[1], spacing[2]}, NmVector3{origin[0], origin[1], origin[2]});

    auto vBounds = volumeBounds<T>(volume, volume->GetLargestPossibleRegion());
    // check if the requested bounds are inside the volume bounds, else fail miserably
    Q_ASSERT(contains(vBounds, iBounds));

    if (!isEquivalent(vBounds, iBounds))
    {
      auto extractor = ExtractFilter::New();
      extractor->SetExtractionRegion(equivalentRegion<T>(volume, inputBounds));
      extractor->SetInput(volume);
      extractor->Update();

      itkImage = extractor->GetOutput();
    }
    else
      itkImage = volume;

    itkImage->DisconnectPipeline();

    auto transform = itk2vtkImageFilter::New();
    transform->SetInput(itkImage);
    transform->Update();

    vtkSmartPointer<vtkImageData> returnImage = vtkImageData::New();
    returnImage->DeepCopy(transform->GetOutput());
    return returnImage;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(VolumetricDataSPtr<T> volume, const Bounds &bounds)
  {
    typename T::Pointer image = volume->itkImage(bounds);
    return vtkImage<T>(image, bounds);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  vtkSmartPointer<vtkImageData> vtkImage(OutputSPtr output, const Bounds &bounds)
  {
    auto volume = volumetricData(output);
    typename T::Pointer image = volume->itkImage(bounds);
    return vtkImage<T>(image, bounds);
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  bool isSegmentationVoxel(const VolumetricDataSPtr<T> volume, const NmVector3 &point)
  {
    Bounds bounds{ '[', point[0], point[0], point[1], point[1], point[2], point[2], ']'};

    bool result = contains(volume->bounds(), bounds, volume->spacing());

    if (result)
    {
      typename T::Pointer voxel = volume->itkImage(bounds);

      result = (SEG_BG_VALUE != *(static_cast<unsigned char*>(voxel->GetBufferPointer())));
    }

    return result;
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  void expandAndDraw(VolumetricDataSPtr<T> volume, typename T::Pointer drawnVolume, Bounds bounds = Bounds())
  {
    if (!bounds.areValid())
    {
      bounds = equivalentBounds<T>(drawnVolume, drawnVolume->GetLargestPossibleRegion());
    }

    volume->resize(boundingBox(bounds, volume->bounds()));
    volume->draw(drawnVolume);
  }



  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::Pointer define_itkImage(const NmVector3             &origin,
                                      const NmVector3             &spacing)
  {
    typename T::PointType   itkOrigin;
    typename T::SpacingType itkSpacing;

    for(int i = 0; i < 3; ++i)
    {
      itkOrigin[i]  = origin[i];
      itkSpacing[i] = spacing[i];
    }

    typename T::Pointer image = T::New();
    // Origin and spacing must be set before calling equivalentRegion on image
    image->SetOrigin(itkOrigin);
    image->SetSpacing(itkSpacing);

    return image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename T::Pointer create_itkImage(const Bounds&                bounds,
                                      const typename T::ValueType  value,
                                      const NmVector3             &spacing,
                                      const NmVector3             &origin)
  {
    typename T::Pointer image = define_itkImage<T>(origin, spacing);

    image->SetRegions(equivalentRegion<T>(image, bounds));
    image->Allocate();
    image->FillBuffer(value);

    return image;
  }

} // namespace EspINA
