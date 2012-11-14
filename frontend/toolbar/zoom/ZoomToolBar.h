/*
 * ZoomToolbar.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ZOOMTOOLBAR_H_
#define ZOOMTOOLBAR_H_

#include <qtoolbar.h>

class ViewManager;
class QAction;

class ZoomToolBar
: public QToolBar
{
  Q_OBJECT

  public:
    explicit ZoomToolBar(ViewManager *vm,
                         QWidget* parent = 0);
    virtual ~ZoomToolBar();

  public slots:
    virtual void ResetViews();
    virtual void InitZoomTool();

  private:
    ViewManager *m_viewManager;
    QAction     *m_resetViews;
    QAction     *m_zoomTool;
};

#endif /* ZOOMTOOLBAR_H_ */
