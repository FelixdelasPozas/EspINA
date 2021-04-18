/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/TemporalStorage.h>
#include <EspinaConfig.h>

// VTK
#include "vtkPolyDataUtils.h"
#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkImageData.h>
#include <vtkImageStencilToImage.h>
#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSmartPointer.h>
#include <vtkDataArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>

// QT
#include <QByteArray>
#include <QString>

// ITK
#include <itkImage.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------------------
QByteArray ESPINA::PolyDataUtils::savePolyDataToBuffer(const vtkSmartPointer<vtkPolyData> polyData)
{
  auto polyWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
  polyWriter->SetInputData(polyData);
  polyWriter->SetFileTypeToBinary();
  polyWriter->SetWriteToOutputString(true);
  polyWriter->Write();

  if (polyWriter->GetErrorCode() != 0)
  {
    auto what    = QObject::tr("Couldn't dump a vtkPolyData data to a buffer, error code: %1.").arg(polyWriter->GetErrorCode());
    auto details = QObject::tr("savePolyDataToBuffer() -> Couldn't dump a vtkPolyData data to a buffer, error code: %1.").arg(polyWriter->GetErrorCode());

    throw EspinaException(what, details);
  }

  return QByteArray(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());
}

//------------------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> ESPINA::PolyDataUtils::readPolyDataFromFile(const QString &fileName)
{
  const auto shortName = getShortFileName(fileName);

  auto reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
  reader->SetFileName(shortName.c_str());
  reader->SetReadAllFields(true);
  reader->Update();

  if (reader->GetErrorCode() != 0)
  {
    auto what    = QObject::tr("Couldn't read a vtkPolyData from file, file: %1, error code: %2.").arg(fileName).arg(reader->GetErrorCode());
    auto details = QObject::tr("readPolyDataFromFile() -> Couldn't read a vtkPolyData from file, file: %1, error code: %2.").arg(fileName).arg(reader->GetErrorCode());

    throw EspinaException(what, details);
  }

  auto data = reader->GetPolyDataOutput();
  data->Modified();

  auto mesh = vtkSmartPointer<vtkPolyData>::New();
  mesh->DeepCopy(data);

  return mesh;
}

//------------------------------------------------------------------------------------
void EspinaCore_EXPORT ESPINA::PolyDataUtils::scalePolyData(vtkSmartPointer<vtkPolyData> polyData, const NmVector3 &ratio)
{
  auto points = polyData->GetPoints();
  double point[3];

  for(vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
  {
    points->GetPoint(i, point);
    for(auto coord: {0,1,2})
    {
      point[coord] = point[coord] * ratio[coord];
    }
    points->SetPoint(i, point);
  }

  points->Modified();
  polyData->Modified();
}

//------------------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> EspinaCore_EXPORT ESPINA::PolyDataUtils::rasterizeContourToVTKImage(vtkPolyData *contour, const Plane plane, const Nm slice, const NmVector3 &spacing)
{
  double bounds[6];
  contour->ComputeBounds();
  contour->GetBounds(bounds);
  auto contourBounds = Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
  auto idx = normalCoordinateIndex(plane);
  contourBounds[2*idx] = contourBounds[(2*idx)+1] = slice;

  // vtkPolyDataToImageStencil filter only works in XY plane so we must rotate the contour to that plane.
  int count = contour->GetPoints()->GetNumberOfPoints();
  auto rotatedContour = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkPoints::New();
  auto lines = vtkCellArray::New();
  vtkIdType index = 0;

  points->SetNumberOfPoints(count);
  vtkIdType numLines = count + 1;

  if (numLines > 0)
  {
    double pos[3];
    auto lineIndices = new vtkIdType[numLines];
    for (int i = 0; i < count; i++)
    {
      contour->GetPoint(i, pos);
      switch(plane)
      {
        case Plane::XY:
          break;
        case Plane::XZ:
          pos[1] = pos[2];
          break;
        case Plane::YZ:
          pos[0] = pos[1];
          pos[1] = pos[2];
          break;
        default:
          Q_ASSERT(false);
          break;
      }
      pos[2] = slice;

      points->InsertPoint(index, pos);
      lineIndices[index] = index;
      index++;
    }

    lineIndices[index] = 0;

    lines->InsertNextCell(numLines, lineIndices);
    delete[] lineIndices;
  }

  rotatedContour->SetPoints(points);
  rotatedContour->SetLines(lines);

  points->Delete();
  lines->Delete();

  rotatedContour->Modified();

  NmVector3 origin{0,0,0};
  auto contourRegion = equivalentRegion<itkVolumeType>(origin, spacing, contourBounds);
  auto contourRegionIndex = contourRegion.GetIndex();
  auto contourRegionSize = contourRegion.GetSize();

  int extent[6];
  extent[0] = contourRegionIndex[0];
  extent[1] = contourRegionIndex[0] + contourRegionSize[0] -1;
  extent[2] = contourRegionIndex[1];
  extent[3] = contourRegionIndex[1] + contourRegionSize[1] -1;
  extent[4] = contourRegionIndex[2];
  extent[5] = contourRegionIndex[2] + contourRegionSize[2] -1;

  // extent and spacing should be changed because vtkPolyDataToImageStencil filter only works in XY plane
  // and we've rotated the contour to that plane
  double temporal;
  double spacingNm[3]{spacing[0], spacing[1], spacing[2]};
  switch(plane)
  {
    case Plane::XY:
      break;
    case Plane::XZ:
      temporal = spacingNm[1];
      spacingNm[1] = spacingNm[2];
      spacingNm[2] = temporal;

      extent[2] = extent[4];
      extent[3] = extent[5];
      break;
    case Plane::YZ:
      temporal = spacingNm[0];
      spacingNm[0] = spacingNm[1];
      spacingNm[1] = spacingNm[2];
      spacingNm[2] = temporal;

      extent[0] = extent[2];
      extent[1] = extent[3];
      extent[2] = extent[4];
      extent[3] = extent[5];
      break;
      default:
      Q_ASSERT(false);
      break;
  }
  extent[4] = contourRegionIndex[idx];
  extent[5] = contourRegionIndex[idx] + contourRegionSize[idx] -1;

  auto polyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
  polyDataToStencil->SetInputData(rotatedContour);
  polyDataToStencil->SetOutputOrigin(0,0,0);
  polyDataToStencil->SetOutputSpacing(spacingNm[0], spacingNm[1], spacingNm[2]);
  polyDataToStencil->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
  polyDataToStencil->SetTolerance(0);
  polyDataToStencil->Update();

  auto stencilToImage = vtkSmartPointer<vtkImageStencilToImage>::New();
  stencilToImage->SetInputConnection(polyDataToStencil->GetOutputPort());
  stencilToImage->SetOutputScalarTypeToUnsignedChar();
  stencilToImage->SetInsideValue(1);
  stencilToImage->SetOutsideValue(0);
  stencilToImage->Update();

  auto image = vtkSmartPointer<vtkImageData>::New();
  image->DeepCopy(stencilToImage->GetOutput());

  return image;
}

//------------------------------------------------------------------------------------
ESPINA::BinaryMaskSPtr<unsigned char> EspinaCore_EXPORT ESPINA::PolyDataUtils::rasterizeContourToMask(vtkPolyData *contour, const Plane plane, const Nm slice, const NmVector3 &spacing)
{
  double bounds[6];
  contour->ComputeBounds();
  contour->GetBounds(bounds);
  auto contourBounds = Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
  auto idx = normalCoordinateIndex(plane);
  contourBounds[2*idx] = contourBounds[(2*idx)+1] = slice;

  auto mask = std::make_shared<BinaryMask<unsigned char>>(contourBounds, spacing);

  auto image = rasterizeContourToVTKImage(contour, plane, slice, spacing);
  int extent[6];
  image->GetExtent(extent);

  BinaryMask<unsigned char>::IndexType imageIndex;
  imageIndex.x = imageIndex.y = imageIndex.z = 0;
  unsigned char *pixel;
  for (int x = extent[0]; x <= extent[1]; ++x)
  {
    for (int y = extent[2]; y <= extent[3]; ++y)
    {
      for (int z = extent[4]; z <= extent[5]; ++z)
      {
        switch(plane)
        {
          case Plane::XY:
            imageIndex.x = x;
            imageIndex.y = y;
            imageIndex.z = z;
            break;
          case Plane::XZ:
            imageIndex.x = x;
            imageIndex.y = z;
            imageIndex.z = y;
            break;
          case Plane::YZ:
            imageIndex.x = z;
            imageIndex.y = x;
            imageIndex.z = y;
            break;
          default:
            Q_ASSERT(false);
            break;
        }

        pixel = reinterpret_cast<unsigned char*>(image->GetScalarPointer(x, y, z));

        if (*pixel == 1)
        {
          mask->setPixel(imageIndex);
        }
      }
    }
  }

  return mask;
}

//------------------------------------------------------------------------------------
VolumeBounds EspinaCore_EXPORT ESPINA::PolyDataUtils::polyDataVolumeBounds(vtkSmartPointer<vtkPolyData> data, const NmVector3& spacing, const NmVector3& origin)
{
  Bounds result;

  if (data && ((data->GetNumberOfCells() > 0) || (data->GetNumberOfPoints() > 0) || (data->GetNumberOfLines() > 0)))
  {
    Nm bounds[6];
  	data->ComputeBounds();
    data->GetBounds(bounds);

    result = Bounds(bounds);
  }

  return VolumeBounds(result, spacing, origin);
}

//------------------------------------------------------------------------------------
QByteArray EspinaCore_EXPORT ESPINA::PolyDataUtils::convertPolyDataToOBJ(const vtkSmartPointer<vtkPolyData> polyData, int startIndex)
{
  auto data = polyData;

  std::stringstream objBuffer;
  objBuffer.precision(6);
  const char NEWLINE('\n');

  //write header
  objBuffer << "# wavefront obj conversion by EspINA " << ESPINA_VERSION << NEWLINE;
  objBuffer << "mtllib NONE" << NEWLINE << NEWLINE;

  vtkSmartPointer<vtkPointData> pntData;
  vtkSmartPointer<vtkPoints> points;
  vtkSmartPointer<vtkDataArray> tcoords;
  int i, i1, i2;
  int idStart = startIndex;
  double p[3];
  vtkCellArray *cells;
  vtkIdType npts = 0;
  vtkIdType *indx = 0;

  // write out the points
  for (i = 0; i < data->GetNumberOfPoints(); i++)
  {
    data->GetPoint(i, p);
    objBuffer << "v " << p[0] << " " << p[1] << " " << p[2] << NEWLINE;
  }

  // write out the point data
  const auto normals = data->GetPointData()->GetNormals();
  if(normals)
  {
    for (i = 0; i < normals->GetNumberOfTuples(); i++)
    {
      normals->GetTuple(i, p);
      objBuffer << "vn " << p[0] << " " << p[1] << " " << p[2] << NEWLINE;
    }
  }

  tcoords = data->GetPointData()->GetTCoords();
  if (tcoords)
  {
    for (i = 0; i < tcoords->GetNumberOfTuples(); i++)
    {
      tcoords->GetTuple(i, p);
      objBuffer << "vt " << p[0] << " " << p[1] << NEWLINE;
    }
  }

  // write out a group name and material
  objBuffer << NEWLINE << "g grp" << idStart << NEWLINE;
  objBuffer << "usemtl mtlNONE" << NEWLINE;

  // write out verts if any
  if (data->GetNumberOfVerts() > 0)
  {
    cells = data->GetVerts();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
    {
      objBuffer << "p ";
      for (i = 0; i < npts; i++)
      {
        objBuffer << static_cast<int>(indx[i])+idStart << " ";
      }
      objBuffer << NEWLINE;
    }
  }

  // write out lines if any
  if (data->GetNumberOfLines() > 0)
  {
    cells = data->GetLines();
    for (cells->InitTraversal(); cells->GetNextCell(npts,indx); )
    {
      objBuffer << "l ";
      if (tcoords)
      {
        for (i = 0; i < npts; i++)
        {
          objBuffer << static_cast<int>(indx[i]) + idStart << "/" << static_cast<int>(indx[i]) + idStart << " ";
        }
      }
      else
      {
        for (i = 0; i < npts; i++)
        {
          objBuffer << static_cast<int>(indx[i]) + idStart << " ";
        }
      }
      objBuffer << NEWLINE;
    }
  }

  // write out polys if any
  if (data->GetNumberOfPolys() > 0)
  {
    cells = data->GetPolys();
    for (cells->InitTraversal(); cells->GetNextCell(npts, indx);)
    {
      objBuffer << "f ";
      for (i = 0; i < npts; i++)
      {
        if (normals)
        {
          if (tcoords)
          {
            objBuffer << static_cast<int>(indx[i]) + idStart << "/" << static_cast<int>(indx[i]) + idStart << "/" << static_cast<int>(indx[i]) + idStart << " ";
          }
          else
          {
            objBuffer << static_cast<int>(indx[i]) + idStart << "//" << static_cast<int>(indx[i]) + idStart << " ";
          }
        }
        else
        {
          if (tcoords)
          {
            objBuffer << static_cast<int>(indx[i]) + idStart << " " << static_cast<int>(indx[i]) + idStart << " ";
          }
          else
          {
            objBuffer << static_cast<int>(indx[i]) + idStart << " ";
          }
        }
      }
      objBuffer << NEWLINE;
    }
  }

  // write out tstrips if any
  if (data->GetNumberOfStrips() > 0)
  {
    cells = data->GetStrips();
    for (cells->InitTraversal(); cells->GetNextCell(npts, indx);)
    {
      for (i = 2; i < npts; i++)
      {
        if (i % 2)
        {
          i1 = i - 1;
          i2 = i - 2;
        }
        else
        {
          i1 = i - 1;
          i2 = i - 2;
        }
        if (normals)
        {
          if (tcoords)
          {
            objBuffer << "f " << static_cast<int>(indx[i1]) + idStart << "/" << static_cast<int>(indx[i1]) + idStart << "/" << static_cast<int>(indx[i1]) + idStart << " ";
            objBuffer << static_cast<int>(indx[i2]) + idStart << "/" << static_cast<int>(indx[i2]) + idStart << "/" << static_cast<int>(indx[i2]) + idStart << " ";
            objBuffer << static_cast<int>(indx[i]) + idStart << "/" << static_cast<int>(indx[i]) + idStart << "/" << static_cast<int>(indx[i]) + idStart << NEWLINE;
          }
          else
          {
            objBuffer << "f " << static_cast<int>(indx[i1]) + idStart << "//" << static_cast<int>(indx[i1]) + idStart << " ";
            objBuffer << static_cast<int>(indx[i2]) + idStart << "//" << static_cast<int>(indx[i2]) + idStart << " ";
            objBuffer << static_cast<int>(indx[i]) + idStart << "//" << static_cast<int>(indx[i]) + idStart << NEWLINE;
          }
        }
        else
        {
          if (tcoords)
          {
            objBuffer << "f " << static_cast<int>(indx[i1]) + idStart << "/" << static_cast<int>(indx[i1]) + idStart << " ";
            objBuffer << static_cast<int>(indx[i2]) + idStart << "/" << static_cast<int>(indx[i2]) + idStart << " ";
            objBuffer << static_cast<int>(indx[i]) + idStart << "/" << static_cast<int>(indx[i]) + idStart << NEWLINE;
          }
          else
          {
            objBuffer << "f " << static_cast<int>(indx[i1]) + idStart << " " << static_cast<int>(indx[i2]) + idStart << " " << static_cast<int>(indx[i]) + idStart << NEWLINE;
          }
        }
      }
    }
  }


  return QByteArray(objBuffer.str().c_str(), objBuffer.str().length());
}
