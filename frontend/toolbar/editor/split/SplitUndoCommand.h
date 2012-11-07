/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef SPLITUNDOCOMMAND_H
#define SPLITUNDOCOMMAND_H

#include <QUndoStack>
#include <model/ModelItem.h>

class Sample;
class Channel;
class EspinaModel;
class Filter;
class Segmentation;
class SplitFilter;

class SplitUndoCommand
: public QUndoCommand
{
public:
  explicit SplitUndoCommand(Segmentation *input,
                            SplitFilter  *filter,
                            EspinaModel  *model);
  virtual ~SplitUndoCommand();

  virtual void redo();
  virtual void undo();

private:
  EspinaModel  *m_model;

  Channel      *m_channel;
  Sample       *m_sample;
  Segmentation *m_seg;
  SplitFilter  *m_filter;
  Segmentation *m_subSeg[2];

  ModelItem::RelationList m_relations;
};

#endif // SPLITUNDOCOMMAND_H
