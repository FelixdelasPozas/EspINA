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

#ifndef IMAGELOGICCOMMAND_H
#define IMAGELOGICCOMMAND_H

#include <QUndoCommand>

#include <Filters/ImageLogicFilter.h>

class EspinaModel;
class Segmentation;

class ImageLogicCommand
: public QUndoCommand
{
  //TODO REVIEW: This class is also defined in RemoveSegmentationCommand
  struct SegInfo
  {
    SegInfo(Segmentation *seg)
    : filter(seg->filter())
    , relations(seg->relations())
    , segmentation(seg){}

    Filter *filter;
    ModelItem::RelationList relations;
    Segmentation *segmentation;
  };

public:
  explicit ImageLogicCommand(QList<Segmentation *> segmentations,
                             ImageLogicFilter::Operation op,
                             EspinaModel *model,
                             TaxonomyElement *taxonomy);

  virtual void redo();
  virtual void undo();

  const QString link(Segmentation *seg);

private:
  EspinaModel *m_model;
  QList<Segmentation *>       m_input;
  ImageLogicFilter::Operation m_op;

  QList<SegInfo>    m_infoList;
  ImageLogicFilter *m_filter;
  Segmentation     *m_seg;
  TaxonomyElement  *m_tax;
};

#endif // IMAGELOGICCOMMAND_H
