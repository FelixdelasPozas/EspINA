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



template<typename T>
typename T::Pointer Testing_Support<T>::Create_Test_Image(const typename T::SizeType size,
                                                          const typename T::ValueType value) {

  typename T::PointType   origin;
  typename T::IndexType   index;
  typename T::SpacingType spacing;

  origin .Fill(0);
  index  .Fill(0);
  spacing.Fill(1);

  return Create_Test_Image(origin, index, size, spacing, value);
  }


  template<typename T>
  typename T::Pointer Testing_Support<T>::Create_Test_Image(const typename T::PointType   origin,
                                                            const typename T::IndexType   index,
                                                            const typename T::SizeType    size,
                                                            const typename T::SpacingType spacing,
                                                            const typename T::ValueType   value) {
    typename T::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    typename T::Pointer image = T::New();
    image->SetRegions(region);
    image->SetOrigin(origin);
    image->SetSpacing(spacing);
    image->Allocate();
    image->FillBuffer(value);

    return image;
}


template<typename T>
bool Testing_Support<T>::Test_Pixel_Values(const typename T::Pointer   image,
                                           const typename T::PixelType value,
                                           const Bounds &bounds) {
  bool pass = true;

  typename T::RegionType region = image->GetLargestPossibleRegion();
  if (bounds.areValid()) {
    region = equivalentRegion<T>(image, bounds);
  }

  typedef itk::ImageRegionIterator<T> ImageIterator;
  ImageIterator it = ImageIterator(image, region);
  it.GoToBegin();
  while (pass && !it.IsAtEnd()) {
    pass = it.Get() == value;
    ++it;
  }

  return pass;
}