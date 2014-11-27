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

#ifndef ESPINA_TOOL_GROUP_H
#define ESPINA_TOOL_GROUP_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include "Tool.h"
#include <Support/ViewManager.h>
#include <GUI/Selectors/Selector.h>

// C++
#include <memory>

// Qt
#include <QAction>

namespace ESPINA
{
  class RenderView;

  class EspinaSupport_EXPORT ToolGroup
  : public QAction
  {
    Q_OBJECT
  public:
    /** \brief ToolGroup class constructor.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] icon icon of the tool group.
     * \param[in] text tooltip of the tool group.
     * \param[in] parent raw pointer of the QObject parent of this one.
     *
     */
    ToolGroup(ViewManagerSPtr viewManager, const QIcon& icon, const QString& text, QObject* parent);

    /** \brief Enables/Disables the tool group.
     * \param[in] value, true to enable false otherwise.
     *
     */
    virtual void setEnabled(bool value) = 0;

    /** \brief Returns true if the tool group is enabled.
     *
     */
    virtual bool enabled() const = 0;

    /** \brief Returns a list of tool smart pointers contained in the tool group.
     *
     */
    virtual ToolSList tools() = 0;

  public slots:
    /** \brief Shows/Hides the tool group tools.
     * \param[in] value true to show false otherwise.
     *
     */
    virtual void showTools(bool value);

  protected:
    ViewManagerSPtr m_viewManager;
  };

  using ToolGroupPtr = ToolGroup *;
} // namespace ESPINA

#endif // ESPINA_TOOL_GROUP_H
