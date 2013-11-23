/*
 * ZoomTool.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ESPINA_ZOOM_AREA_H
#define ESPINA_ZOOM_AREA_H

#include <Support/Tool.h>
#include <Support/ViewManager.h>

class QCursor;

namespace EspINA
{
  class ZoomSelectionWidget;

  class ZoomArea
  : public Tool
  {
  public:
    ZoomArea(ViewManagerSPtr viewManager);
    virtual ~ZoomArea();

    virtual bool enabled() const;

    virtual void setEnabled(bool value);

    virtual QList<QAction *> actions() const;

  private:
    bool m_enabled;
    QAction *m_zoomArea;
    ZoomSelectionWidget *m_widget;
    ViewManagerSPtr m_viewManager;
    QCursor zoomCursor;
  };

  using ZoomAreaSPtr = std::shared_ptr<ZoomArea>;

} // namespace EspINA

#endif /* ZOOMTOOL_H_ */
