/*
 * ContourInspectorWidget.cpp
 *
 *  Created on: Jan 14, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "ContourInspectorWidget.h"
#include <Filters/ContourSource.h>

using namespace EspINA;

//----------------------------------------------------------------------------
ContourFilterInspector::Widget::Widget(ContourSource* source)
    : m_source(source)
{
  setupUi(this);

  UpdateValues();
  
  this->editButton->setEnabled(false);
  
  connect(this->editButton, SIGNAL(clicked(bool)), this, SLOT(EditContours()));
  connect(m_source, SIGNAL(modified(ModelItem*)), this, SLOT(UpdateValues()));
}

 //----------------------------------------------------------------------------
ContourFilterInspector::Widget::~Widget()
{
}

 //----------------------------------------------------------------------------
void ContourFilterInspector::Widget::UpdateValues()
{
int total_XY = this->m_source->m_contourMap[AXIAL].size();
int total_XZ = this->m_source->m_contourMap[CORONAL].size();
int total_YZ = this->m_source->m_contourMap[SAGITTAL].size();

this->totalXY->setText(QString().setNum(total_XY, 10));
this->totalXZ->setText(QString().setNum(total_XZ, 10));
this->totalYZ->setText(QString().setNum(total_YZ, 10));
this->totalContours->setText(QString().setNum(total_XY + total_XZ + total_YZ, 10));
}

 //----------------------------------------------------------------------------
void ContourFilterInspector::Widget::EditContours()
{
// TODO
}
