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

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include <QVariant>

namespace EspINA
{
  class EspinaCore_EXPORT SegmentationExtension
  {
  public:
    using Type        = QString;
    using InfoTag     = QString;
    using InfoTagList = QList<InfoTag>;

  public:
    virtual ~SegmentationExtension() {}

    virtual void initialize() = 0;

    virtual void invalidate() = 0;

    void setSegmentation(SegmentationPtr seg)
    { m_segmentation = seg; onSegmentationSet(seg); }

    SegmentationPtr segmentation() {return m_segmentation;}

    virtual void onSegmentationSet(SegmentationPtr seg) = 0;

    virtual bool validCategory(const QString &classificationName) const = 0;

    virtual InfoTagList availableInformations() const = 0;

    virtual QVariant information(const InfoTag &tag) = 0;

    virtual QString toolTipText() const
    { return QString(); }

  protected:
    SegmentationExtension() : m_segmentation{nullptr} {}

    SegmentationPtr m_segmentation;
    bool            m_enabled;
  };

  using SegmentationExtensionPtr   = SegmentationExtension *;
  using SegmentationExtensionList  = QList<SegmentationExtensionPtr>;
  using SegmentationExtensionSPtr  = std::shared_ptr<SegmentationExtension>;
  using SegmentationExtensionSList = QList<SegmentationExtensionSPtr>;
} // namespace EspINA

#endif // SEGMENTATIONEXTENSION_H
