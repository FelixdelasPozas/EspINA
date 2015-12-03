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

//-----------------------------------------------------------------------------
template<typename T>
typename T::RegionType equivalentRegion(const T* image, const Bounds& bounds)
{
  typename T::PointType   o = image->GetOrigin();
  typename T::SpacingType s = image->GetSpacing();

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

  // TODO: as stupid as it sounds this happens on some coordinates when
  // using the brush and painting outside the view. Investigate and fix.
  for(auto i: {0,1,2})
  {
    if(i0[i] > i1[i])
    {
      std::swap(i0[i], i1[i]);
    }
  }

  typename T::RegionType region;
  region.SetIndex(i0);
  region.SetUpperIndex(i1);

  for (auto i: {0,1,2})
  {
    if (region.GetSize(i) == 0)
    {
      region.SetSize(i,1);
    }
  }

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
Bounds equivalentBounds(const typename T::Pointer image)
{
  return equivalentBounds<T>(image, image->GetLargestPossibleRegion());
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

  return VolumeBounds(bounds, spacing, origin);
}

//-----------------------------------------------------------------------------
template<typename T>
VolumeBounds volumeBounds(const typename T::Pointer image)
{
  return volumeBounds<T>(image, equivalentBounds<T>(image, image->GetLargestPossibleRegion()));
}

//-----------------------------------------------------------------------------
template<typename T>
VolumeBounds volumeBounds(const typename T::Pointer image, const typename T::RegionType& region)
{
  return volumeBounds<T>(image, equivalentBounds<T>(image, region));
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds leftSliceBounds(const T &volume)
{
  Bounds slice = volume->bounds();
  auto spacing = volume->bounds().spacing();

  slice[1] = slice[0] + spacing[0]/2.0;

  return slice;
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds rightSliceBounds(const  T &volume)
{
  Bounds slice = volume->bounds();
  auto spacing = volume->bounds().spacing();

  slice[0] = slice[1] - spacing[0]/2.0;

  return slice;
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds topSliceBounds(const  T &volume)
{
  Bounds slice = volume->bounds();
  auto spacing = volume->bounds().spacing();

  slice[3] = slice[2] + spacing[1]/2.0;

  return slice;
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds bottomSliceBounds(const  T &volume)
{
  Bounds slice = volume->bounds();
  auto spacing = volume->bounds().spacing();

  slice[2] = slice[3] - spacing[1]/2.0;

  return slice;
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds frontSliceBounds(const  T &volume)
{
  Bounds slice = volume->bounds();
  auto spacing = volume->bounds().spacing();

  slice[5] = slice[4] + spacing[2]/2.0;

  return slice;
}

//-----------------------------------------------------------------------------
template<typename T>
Bounds backSliceBounds(const  T &volume)
{
  Bounds slice = volume->bounds();
  auto spacing = volume->bounds().spacing();

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
Bounds minimalBounds(const typename T::Pointer image, const typename T::ValueType bgValue)
{
  Bounds bounds;

  itk::ImageRegionConstIterator<T> it(image, image->GetLargestPossibleRegion());
  auto origin  = image->GetOrigin();
  auto spacing = image->GetSpacing();

  it.GoToBegin();
  while (!it.IsAtEnd())
  {
    if (it.Get() != bgValue)
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
typename T::PointType ItkPoint(const NmVector3& point)
{
  typename T::PointType itkPoint;

  for(int i = 0; i < 3; ++i)
  {
    itkPoint[i] = point[i];
  }

  return itkPoint;
}

//-----------------------------------------------------------------------------
template<typename T>
typename T::SpacingType ItkSpacing(const NmVector3& spacing)
{
  typename T::SpacingType itkSpacing;

  for(int i = 0; i < 3; ++i)
  {
    itkSpacing[i] = spacing[i];
  }

  return itkSpacing;
}


//-----------------------------------------------------------------------------
template<typename T>
NmVector3 ToNmVector3(typename T::SpacingType itkSpacing)
{
  NmVector3 vector;

  for(int i = 0; i < 3; ++i)
  {
    vector[i] = itkSpacing[i];
  }

  return vector;
}

//-----------------------------------------------------------------------------
template<typename T>
NmVector3 ToNmVector3(typename T::PointType itkPoint)
{
  NmVector3 vector;

  for(int i = 0; i < 3; ++i)
  {
    vector[i] = itkPoint[i];
  }

  return vector;
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
