/*
 * ZoomTool.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ESPINA_ZOOM_TOOL_H
#define ESPINA_ZOOM_TOOL_H

#include <Support/Tool.h>

class QCursor;

namespace EspINA
{
  class ZoomSelectionWidget;

  class ZoomTool
  : public Tool
  {
  public:
    explicit ZoomTool(ViewManager *);
    virtual ~ZoomTool();

    // implements ITool
    virtual QCursor cursor() const;
    virtual bool filterEvent(QEvent *e, EspinaRenderView *view=NULL);
    virtual void setInUse(bool value);
    virtual void setEnabled(bool value);
    virtual bool enabled() const;

  private:
    bool m_enabled;
    bool m_inUse;
    ZoomSelectionWidget *m_widget;
    ViewManager *m_viewManager;
    QCursor zoomCursor;
  };

  typedef boost::shared_ptr<ZoomTool> ZoomToolSPtr;

} // namespace EspINA

#endif /* ZOOMTOOL_H_ */
