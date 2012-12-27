/*
 * HueSelector.h
 *
 *  Created on: August 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef HUESELECTOR_H_
#define HUESELECTOR_H_

// Qt includes
#include <QWidget>

namespace EspINA
{
class HueSelector
: public QWidget
{
Q_OBJECT
public:
  HueSelector(QWidget* parent = 0);
  ~HueSelector();

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

} // namespace EspINA

#endif // HUESELECTOR_H_
