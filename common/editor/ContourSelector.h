/*
 * ContourSelection.h
 *
 *  Created on: Aug 28, 2012
 *      Author: Félix de las Pozas Alvarez
 */

#ifndef _CONTOURSELECTION_H_
#define _CONTOURSELECTION_H_

#include <common/selection/SelectionHandler.h>

class QCursor;

class ContourSelector
: public IPicker
{
public:
  explicit ContourSelector(IPicker *successor = NULL);
  virtual ~ContourSelector();

  virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);
  virtual QCursor cursor();
private:
  QCursor m_cursor;
};

#endif /* _CONTOURSELECTION_H_ */
