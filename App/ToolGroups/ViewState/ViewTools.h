/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <@>

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

#ifndef ESPINA_ZOOM_TOOLS_H
#define ESPINA_ZOOM_TOOLS_H

// ESPINA
#include <Support/Widgets/ToolGroup.h>
#include "ToggleSegmentationsVisibility.h"
#include "ResetZoom.h"
#include "ZoomArea.h"
#include "ToggleCrosshairVisibility.h"

// Qt
#include <QShortcut>

namespace ESPINA
{
  class ViewTools
  : public ToolGroup
  {
    Q_OBJECT

  public:
    /** brief ViewTools class constructor.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] parent, raw pointer to the QWidget parent of this object.
     */
    explicit ViewTools(ViewManagerSPtr viewManager, QWidget *parent = 0);

    /** brief ViewTools class virtual destructor.
     *
     */
    virtual ~ViewTools();

    /** brief Implements ToolGroup::setEnabled().
     *
     */
    virtual void setEnabled(bool value);

    /** brief Implements ToolGroup::enabled().
     *
     */
    virtual bool enabled() const;

    /** brief Implements ToolGroup::tools().
     *
     */
    virtual ToolSList tools();

  public slots:
  	/** brief Aborts current operation.
  	 *
  	 */
    void abortOperation();

  private:
    ToggleSegmentationsVisibilitySPtr m_toggleSegmentations;
    ToggleCrosshairVisibilitySPtr     m_toggleCrosshair;
    ResetZoomSPtr                     m_resetZoom;
    ZoomAreaSPtr                      m_zoomArea;

    bool m_enabled;

    QShortcut *m_segmentationsShortcut;
    QShortcut *m_crosshairShortcut;
  };

} // namespace ESPINA

#endif // ESPINA_ZOOM_TOOLS_H
