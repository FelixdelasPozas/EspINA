/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SEGMENTATIONEXTENSION_H
#define SEGMENTATIONEXTENSION_H

#include "Core/Model/Segmentation.h"
#include "Core/Extensions/ModelItemExtension.h"

namespace EspINA
{

  class Segmentation::Extension
  : public ModelItem::Extension
  {
  public:
    virtual ~Extension() {}
  protected:
    Extension() : m_segmentation(NULL){}
    SegmentationPtr m_segmentation;
  };


  /// Interface to extend segmentation's behaviour
  class Segmentation::Information
  : public Segmentation::Extension
  {
    Q_OBJECT
  public:
    virtual ~Information(){}

    virtual void setSegmentation(SegmentationPtr seg);

    virtual SegmentationPtr segmentation() {return m_segmentation;}

    virtual Segmentation::InfoTagList availableInformations() const = 0;

    virtual QVariant information(const Segmentation::InfoTag &tag) = 0;

    /// Prototype
    virtual Segmentation::InformationExtension clone() = 0;

    virtual void initialize() = 0;

  public slots:
    /// Invalidates segmentation's extension. If no segmentation is given
    /// invalidate this extension for its segmentation
    virtual void invalidate(SegmentationPtr segmentation = NULL) = 0;

  protected:
    Information() {}
  };

} // namespace EspINA

#endif // SEGMENTATIONEXTENSION_H
