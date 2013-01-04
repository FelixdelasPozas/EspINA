/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
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


#ifndef SPLITUNDOCOMMAND_H
#define SPLITUNDOCOMMAND_H

#include <QUndoStack>
#include <Core/Model/ModelItem.h>
#include <Core/Model/EspinaModel.h>

namespace EspINA
{
  class SplitFilter;

  class SplitUndoCommand
  : public QUndoCommand
  {
  public:
    static const Filter::FilterType FILTER_TYPE;

  public:
    explicit SplitUndoCommand(SegmentationSPtr input,
                              FilterSPtr       filter,
                              SegmentationSPtr splitSeg[2],
                              EspinaModel *model);
    virtual ~SplitUndoCommand();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;

    ChannelSPtr m_channel;
    SampleSPtr       m_sample;
    SegmentationSPtr m_seg;
    FilterSPtr       m_filter;
    SegmentationSPtr m_subSeg[2];

    RelationList m_relations;
  };

} // namespace EspINA

#endif // SPLITUNDOCOMMAND_H
