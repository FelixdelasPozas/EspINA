/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include <Core/Utils/BinaryMask.h>
#include <Core/Utils/Bounds.h>
#include <Core/EspinaTypes.h>

#include <itkImageRegionIterator.h>

#include <QString>
#include <QDebug>

using namespace EspINA;

using BMask = BinaryMask<unsigned char>;

int binaryMask_itkImage_constructor(int argc, char** argv)
{
  bool error = false;

  BMask::itkIndex index;
  index[0] = 1;
  index[1] = 2;
  index[2] = 3;
  BMask::itkSize size;
  size[0] = 5;
  size[1] = 5;
  size[2] = 5;
  BMask::itkRegion region;
  region.SetIndex(index);
  region.SetSize(size);
  BMask::itkSpacing spacing;
  spacing[0] = 3;
  spacing[1] = 4;
  spacing[2] = 5;

  BMask::itkImageType::Pointer image = BMask::itkImageType::New();
  image->SetRegions(region);
  image->SetSpacing(spacing);
  image->Allocate();
  image->FillBuffer(0);

  int i = 0;
  QString itkVolumeValues;
  itk::ImageRegionIterator<BMask::itkImageType> it(image, region);
  while (!it.IsAtEnd())
  {
    if (i % 2)
      it.Set(1);

    itkVolumeValues += QString::number(it.Get());
    ++i;
    ++it;
  }

  BMask *mask = new BMask(image, 0);

  NmVector3 otherSpacing(mask->spacing());
  error |= ((otherSpacing[0] != spacing[0]) && (otherSpacing[1] != spacing[1]) && (otherSpacing[2] != spacing[2]));

  Bounds bounds = mask->bounds();
  error |= ((index[0] * spacing[0] != bounds[0]) && ((index[0]+size[0]) * spacing[0] != bounds[1]) &&
            (index[1] * spacing[1] != bounds[2]) && ((index[1]+size[1]) * spacing[1] != bounds[3]) &&
            (index[2] * spacing[2] != bounds[4]) && ((index[2]+size[2]) * spacing[2] != bounds[5]));

  mask->setForegroundValue(1);

  QString maskVolumeValues;
  BMask::const_iterator ri(mask);
  int b = 0;
  while(!ri.isAtEnd())
  {
    maskVolumeValues += QString::number(ri.Get());
    ++ri;
    ++b;
  }

  error |= (itkVolumeValues != maskVolumeValues);

  return error;
}

