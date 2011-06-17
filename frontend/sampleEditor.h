/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
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

#ifndef SAMPLEEDITOR_H
#define SAMPLEEDITOR_H

#include <QDialog>
#include "ui_sampleEditor.h"

class Sample;

class SampleEditor : public QDialog, private Ui::SampleEditor
{
 Q_OBJECT
public:
  SampleEditor(QWidget* parent = 0, Qt::WindowFlags f = 0);
    
  virtual void setSample(Sample *sample);
  void spacing(double value[3]);

public slots:
  void unitChanged(int unitIndex);
  void updateSpacing();
  
protected:
  virtual void enterEvent(QEvent *event);
  virtual void leaveEvent(QEvent *event);
  
private:
  Sample* m_sample;
};
#endif // SAMPLEEDITOR_H
