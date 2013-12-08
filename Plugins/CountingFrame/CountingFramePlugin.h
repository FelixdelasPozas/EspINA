/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_COUNTING_FRAME_PLUGIN_H
#define ESPINA_COUNTING_FRAME_PLUGIN_H

#include "CountingFramePlugin_Export.h"

#include <Support/Plugin.h>

#include "CountingFrameManager.h"

namespace EspINA
{
  namespace CF
  {
    class CountingFramePlugin_EXPORT CountingFramePlugin
    : public Plugin
    {
      Q_OBJECT
      Q_INTERFACES(EspINA::Plugin)

    public:
      explicit CountingFramePlugin();
      virtual ~CountingFramePlugin();

      virtual void init(ModelAdapterSPtr model,
                        ViewManagerSPtr  viewManager,
                        QUndoStack      *undoStack);

      virtual QList< DockWidget* > dockWidgets();

      //virtual EngineList colorEngines();
    private:
      CountingFrameManager m_manager;
      ModelAdapterSPtr     m_model;
      ViewManagerSPtr      m_viewManager;
      QUndoStack          *m_undoStack;
    };
  }
} // namespace EspINA

#endif // ESPINA_COUNTING_FRAME_PLUGIN_H
