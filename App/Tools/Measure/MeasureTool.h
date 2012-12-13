/*
 * MeasureTool.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef MEASURETOOL_H_
#define MEASURETOOL_H_

// EspINA
#include <GUI/Tools/ITool.h>

class ViewManager;
class MeasureWidget;
class QCursor;

class MeasureTool
: public ITool
{
  public:
    explicit MeasureTool(ViewManager *);
    virtual ~MeasureTool();

    // implements ITool
    virtual QCursor cursor() const { return Qt::CrossCursor; };
    virtual bool filterEvent(QEvent *e, EspinaRenderView *view=NULL);
    virtual void setInUse(bool value);
    virtual void setEnabled(bool value);
    virtual bool enabled() const;

  private:
    bool m_enabled;
    bool m_inUse;
    MeasureWidget *m_widget;
    ViewManager *m_viewManager;
};

#endif /* MEASURETOOL_H_ */
