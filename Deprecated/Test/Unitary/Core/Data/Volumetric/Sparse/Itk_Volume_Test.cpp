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

#include "Itk_Volume_Test.h"

#include <unitary/Testing_Support.h>

using namespace ESPINA;

bool Itk_Volume_Test::SameLargestRegion(const ItkVolume &volume, const ImageType::Pointer image) {

  ImageType::Pointer volumeImage = volume.itkImage();

  return volumeImage->GetLargestPossibleRegion() == image->GetLargestPossibleRegion();
}


bool Itk_Volume_Test::SamePixelValues(const ItkVolume &volume, const ImageType::Pointer image) {
  bool passes = true;

  ImageType::Pointer volumeImage = volume.itkImage();

  ImageIterator iit = ImageIterator(image, image->GetLargestPossibleRegion());
  ImageIterator vit = ImageIterator(volumeImage, volumeImage->GetLargestPossibleRegion());

  iit.GoToBegin(); vit.GoToBegin();
  while (passes && !iit.IsAtEnd()) {
    if (iit.Get() != vit.Get()) {
      passes = false;
    }
    ++iit;
    ++vit;
  }

  return passes;
}


bool Itk_Volume_Test::SameMemoryAllocated(const ItkVolume &volume, const ImageType::Pointer image) {
  ImageType::SizeType size    = image->GetLargestPossibleRegion().GetSize();
  double              memory  = memory_size_in_MB(size[0]*size[1]*size[2]);

  return memory == volume.memoryUsage();
}