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


#ifndef ADDFILTER_H
#define ADDFILTER_H

#include "EspinaUndo_Export.h"

// EspINA
#include "Core/Model/Filter.h"
#include <Core/Model/EspinaModel.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class EspinaUndo_EXPORT AddFilter
  : public QUndoCommand
  {
    public:
	  explicit AddFilter(FilterSPtr   filter,
						 EspinaModel  *model,
						 QUndoCommand *parent = 0);

	  virtual void redo();
	  virtual void undo();

	private:
	  EspinaModel *m_model;

	  FilterSPtr m_filter;
  };

} // namespace EspINA

#endif // ADDFILTER_H
