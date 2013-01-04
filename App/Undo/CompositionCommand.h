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

#ifndef COMPOSITIONCOMMAND_H
#define COMPOSITIONCOMMAND_H

#include <QUndoCommand>

#include <Filters/ImageLogicFilter.h>
#include <Core/Model/EspinaModel.h>

namespace EspINA
{

  class CompositionCommand
  : public QUndoCommand
  {
    struct SegInfo
    {
      SegInfo(SegmentationSPtr seg)
      : filter(seg->filter())
      , relations(seg->relations())
      , segmentation(seg){}

      FilterSPtr filter;
      RelationList relations;
      SegmentationSPtr segmentation;
    };

  public:
    static const Filter::FilterType FILTER_TYPE;

  public:
    explicit CompositionCommand(const SegmentationList &segmentations,
                                TaxonomyElementSPtr      taxonomy,
                                EspinaModel          *model);

    virtual void redo();
    virtual void undo();

    const QString link(SegmentationSPtr seg);

  private:
    EspinaModel *m_model;

    SegmentationSList   m_input;
    TaxonomyElementSPtr m_tax;

    QList<SegInfo>  m_infoList;
    FilterSPtr       m_filter;
    SegmentationSPtr m_seg;
  };

} // namespace EspINA

#endif // COMPOSITIONCOMMAND_H
