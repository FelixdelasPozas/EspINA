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


#ifndef ADDSEGMENTATION_H
#define ADDSEGMENTATION_H

#include <QSharedPointer>
#include <QUndoCommand>

#include "common/model/Filter.h"
#include "common/model/Segmentation.h"

class AddSegmentation : public QUndoCommand
{
public:
  explicit AddSegmentation(Segmentation *seg,
			   QUndoCommand *parent=0);
  virtual void redo();
  virtual void undo();

private:
  Segmentation *m_segmentation;
};

#endif // ADDSEGMENTATION_H
