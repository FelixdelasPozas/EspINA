/*
 * HueSelector.h
 *
 *  Created on: August 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef _HueSelector_H_
#define _HueSelector_H_

// Qt includes
#include <QtGui>
#include <ui_HueSelector.h>

class QtHuePicker : public QWidget
{
Q_OBJECT
public:
  QtHuePicker(QWidget* parent = 0);
  ~QtHuePicker();

public slots:
  void setHueValue(int h);

signals:
  void newHsv(int h, int s, int v);

protected:
  void paintEvent(QPaintEvent*);
  void mouseMoveEvent(QMouseEvent *);
  void mousePressEvent(QMouseEvent *);

private:
  int val;
  int hue;
  int sat;

  int x2val(int y);
  int val2x(int val);
  void setVal(int v);

  QPixmap *pix;
};

class HueSelector : public QDialog, private Ui::HueSelector
{
Q_OBJECT
public:
  // constructor & destructor
  HueSelector(double, QWidget *parent = 0, Qt::WindowFlags f = Qt::Dialog);
  ~HueSelector();

  bool ModifiedData();
  int GetHueValue();
  void SetHueValue(int h);

public slots:
  void AcceptedData();
  void newHsv(int,int,int);
  void spinboxChanged(int);

private slots:

private:
  bool modified;
  QtHuePicker *hp;
  int hue;
};

#endif // _QTLABELEDITOR_H_
