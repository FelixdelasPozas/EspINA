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

#ifndef ESPINA_TOOL_H
#define ESPINA_TOOL_H

#include "Support/EspinaSupport_Export.h"

// C++
#include <memory>

// Qt
#include <QObject>
#include <QCursor>

class QEvent;
class QAction;

namespace ESPINA
{
  class RenderView;

  class EspinaSupport_EXPORT Tool
  : public QObject
  {
    public:
  		/** brief Enables/Disables the tool.
  		 * \param[in] value, true to enable false otherwise.
  		 *
  		 */
      virtual void setEnabled(bool value) = 0;

      /** brief Returns true if the tool is enabled, false otherwise.
       *
       */
      virtual bool enabled() const = 0;

      /** brief Returns a list of QActions provided by the tool.
       *
       */
      virtual QList<QAction *> actions() const = 0;

    signals:
      void changedActions();
  };

  using ToolSPtr  = std::shared_ptr<Tool>;
  using ToolSList = QList<ToolSPtr>;
} // namespace ESPINA

#endif // ESPINA_TOOL_H
