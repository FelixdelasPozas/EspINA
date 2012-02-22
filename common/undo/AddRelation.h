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


#ifndef ADDRELATION_H
#define ADDRELATION_H

#include <QUndoCommand>
#include "common/model/ModelItem.h"

class ModelItem;

class AddRelation
: public QUndoCommand
{
public:
  explicit AddRelation(ModelItem *ancestor,
		       ModelItem *successor,
		       const QString description,
		       QUndoCommand* parent = 0);

  explicit AddRelation(ModelItemPtr ancestor,
		       ModelItemPtr successor,
		       const QString description,
		       QUndoCommand* parent = 0);

  virtual void redo();
  virtual void undo();

private:
  ModelItem    *m_ancester;
  ModelItem    *m_succesor;
  const QString m_description;
};

#endif // ADDRELATION_H
