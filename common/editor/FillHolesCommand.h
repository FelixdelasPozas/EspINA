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


#ifndef FILLHOLESCOMMAND_H
#define FILLHOLESCOMMAND_H

#include <QUndoStack>

#include <model/Segmentation.h>

class FillHolesCommand
: public QUndoCommand
{
public:
  explicit FillHolesCommand(SegmentationList inputs);
  virtual ~FillHolesCommand();

  virtual void redo();
  virtual void undo();

private:
  typedef QPair<Filter *, unsigned int> Connection;

  SegmentationList  m_segmentations;
  QList<Connection> m_oldConnections, m_newConnections;
};

#endif // FILLHOLESCOMMAND_H
