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


#ifndef SEGMENTATIONEXTENSION_H
#define SEGMENTATIONEXTENSION_H

#include "Core/Extensions/ModelItemExtension.h"

#include <QSharedPointer>

namespace EspINA
{
  class Segmentation;
  class SegmentationRepresentation;
  typedef QSharedPointer<SegmentationRepresentation> SegmentationRepresentationPtr;

  /// Interface to extend segmentation's behaviour
  class SegmentationExtension
  : public ModelItemExtension
  {
  public:
    virtual ~SegmentationExtension(){}

    virtual void setSegmentation(SegmentationPtr seg) {m_seg = seg;}

    virtual SegmentationRepresentationPtr representation(QString rep) = 0;

    virtual SegmentationPtr segmentation() {return m_seg;}

    /// Prototype
    virtual SegmentationExtensionPtr clone() = 0;

  protected:
    SegmentationExtension() : m_seg(NULL){}
    SegmentationPtr m_seg;
  };

} // namespace EspINA

#endif // SEGMENTATIONEXTENSION_H
