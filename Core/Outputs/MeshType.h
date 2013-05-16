/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef MESHTYPE_H
#define MESHTYPE_H

#include <Core/Model/OutputRepresentation.h>

namespace EspINA
{
  class MeshType
  : public SegmentationRepresentation
  {
  public:
    static const FilterOutput::OutputRepresentationName TYPE;

  public:
    explicit MeshType(itkVolumeType::SpacingType spacing, FilterOutput *output = NULL)
    : SegmentationRepresentation(output)
    , m_spacing(spacing) {}

    virtual FilterOutput::OutputRepresentationName type() const
    { return TYPE; }

    virtual void addEditedRegion(const EspinaRegion &region, int cacheId = -1) {}

    virtual vtkAlgorithmOutput *mesh() = 0;

    itkVolumeType::SpacingType spacing() const
    { return m_spacing; }

  protected:
    static QString cachePath(const QString &fileName)
    { return QString("%1/%2").arg(MeshType::TYPE).arg(fileName); }

  protected:
    itkVolumeType::SpacingType m_spacing;
  };

  typedef boost::shared_ptr<MeshType> MeshTypeSPtr;

  MeshTypeSPtr meshOutput(SegmentationOutputSPtr output);
} // namespace EspINA

#endif // MESHTYPE_H
