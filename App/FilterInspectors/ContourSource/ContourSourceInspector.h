/*
 * ContourSourceInspector.h
 *
 *  Created on: Sep 30, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef CONTOURSOURCEINSPECTOR_H_
#define CONTOURSOURCEINSPECTOR_H_

#include <QWidget>

#include "ui_ContourSourceInspector.h"

#include <Filters/ContourSource.h> // NOTE Rquiere linkar gui con filter, alguien mas lo necesita??

class ContourSource;

class ContourSource::ContourSourceInspector
: public QWidget
, public Ui::ContourSourceInspector
{
  Q_OBJECT
public:
  explicit ContourSourceInspector(ContourSource *source);
  virtual ~ContourSourceInspector();

protected slots:
  void EditContours();
  void UpdateValues();

private:
  ContourSource *m_source;
};



#endif /* CONTOURSOURCEINSPECTOR_H_ */
