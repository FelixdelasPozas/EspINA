/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
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
 */

#ifndef IMAGELOGICCOMMAND_H
#define IMAGELOGICCOMMAND_H

#include <QUndoCommand>

#include <Filters/ImageLogicFilter.h>

namespace EspINA
{
  class ImageLogicCommand
  : public QUndoCommand
  {
    //TODO REVIEW: This class is also defined in RemoveSegmentationCommand
    struct SegInfo
    {
      SegInfo(SegmentationPtr seg)
      : filter(seg->filter())
      , relations(seg->relations())
      , segmentation(seg){}

      FilterPtr filter;
      ModelItem::RelationList relations;
      SegmentationPtr segmentation;
    };

  public:
    explicit ImageLogicCommand(SegmentationList  segmentations,
                               ImageLogicFilter::Operation op,
                               EspinaModelPtr model,
                               TaxonomyElementPtr taxonomy);

    virtual void redo();
    virtual void undo();

    const QString link(SegmentationPtr seg);

  private:
    EspinaModelPtr m_model;

    SegmentationList m_input;
    ImageLogicFilter::Operation m_op;

    QList<SegInfo>      m_infoList;
    ImageLogicFilter   *m_filter;
    SegmentationPtr     m_seg;
    TaxonomyElementPtr  m_tax;
  };

} // namespace EspINA

#endif // IMAGELOGICCOMMAND_H
