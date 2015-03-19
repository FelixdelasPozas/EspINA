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

#ifndef ESPINA_ZOOM_AREA_H
#define ESPINA_ZOOM_AREA_H

// ESPINA
#include <GUI/View/EventHandler.h>
#include <Support/Widgets/Tool.h>
#include <Support/ViewManager.h>
#include <GUI/View/Widgets/EspinaWidget.h>

class QCursor;

namespace ESPINA
{
  class ZoomSelectionWidget;

  class ZoomAreaTool
  : public Tool
  {
    Q_OBJECT
  public:
    /** \brief ZoomArea class constructor.
     * \param[in] viewManager, view manager smart pointer.
     */
    explicit ZoomAreaTool(ViewManagerSPtr viewManager);

    /** \brief ZoomArea class destructor.
     *
     */
    virtual ~ZoomAreaTool();

    virtual QList<QAction *> actions() const;

    /** \brief Cancels current operation.
     *
     */
    void abortOperation();

  public slots:
    /** \brief Initializes the tool (inserts the widget in the view manager and sets the event handler).
     *
     */
    void activateTool(bool value);

  private:
    virtual void onToolEnabled(bool enabled) override;

  private:
    ViewManagerSPtr      m_viewManager;
    QAction             *m_zoomArea;
    EspinaWidgetSPtr     m_widget;
    EventHandlerSPtr     m_zoomHandler;
  };

  using ZoomAreaToolSPtr = std::shared_ptr<ZoomAreaTool>;

} // namespace ESPINA

#endif /* ZOOMTOOL_H_ */
