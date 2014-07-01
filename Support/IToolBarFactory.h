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

#ifndef ITOOLBARFACTORY_H
#define ITOOLBARFACTORY_H

#include "EspinaCore_Export.h"

#include "Core/Interfaces/IToolBar.h"
// Qt

class QUndoStack;

namespace EspINA
{
  class ModelAdapter;
  class ViewManager;

  class EspinaCore_EXPORT IToolBarFactory
  {
  public:
    virtual void initToolBarFactory(ModelAdapter *model,
                                    QUndoStack  *undoStack,
                                    ViewManager *viewManager) = 0;
    virtual ~IToolBarFactory();

    virtual QList<IToolBar *> toolBars() const = 0;
  };

} // namespace EspINA

Q_DECLARE_INTERFACE(EspINA::IToolBarFactory,
                    "es.upm.cesvima.EspINA.ToolBarInterface/1.4")

#endif //ITOOLBARFACTORY_H
