/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef ADDSAMPLE_H
#define ADDSAMPLE_H

#include "Undo/EspinaUndo_Export.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>

// Qt
#include <QUndoStack>

namespace EspINA
{
  class EspinaUndo_EXPORT AddSample
  : public QUndoCommand
  {
  public:
    explicit AddSample(SampleSPtr    sample,
                       EspinaModel  *model,
                       QUndoCommand *parent = NULL);
    virtual ~AddSample();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;

    SampleSPtr m_sample;
  };

}// namespace EspINA

#endif // ADDSAMPLE_H
