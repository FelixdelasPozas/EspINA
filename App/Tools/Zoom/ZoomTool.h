/*
 * ZoomTool.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ZOOMTOOL_H_
#define ZOOMTOOL_H_

#include <GUI/Tools/ITool.h>

class QCursor;

namespace EspINA
{
  class ViewManager;
  class ZoomSelectionWidget;

  class ZoomTool
  : public ITool
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

} // namespace EspINA

#endif /* ZOOMTOOL_H_ */
