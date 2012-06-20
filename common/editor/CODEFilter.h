/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#ifndef CODEFILTER_H
#define CODEFILTER_H

#include <model/Filter.h>

static const QString CODE = "EditorToolBar::CODEFilter";

class CODEFilter
: public Filter
{
public:
  enum Operation
  {
    CLOSE,
    OPEN,
    DILATE,
    ERODE
  };

  class CODEArguments;

public:
  explicit CODEFilter(Segmentation *seg, Operation op, unsigned int radius);
  explicit CODEFilter(Arguments args);
  virtual ~CODEFilter();

  void run();

  /// Implements Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role) const;
  virtual QString serialize() const;

  /// Implements Filter Interface
  virtual int numProducts() const;
  virtual Segmentation* product(int index) const;

  virtual pqData preview();
  virtual QWidget* createConfigurationWidget();

private:
  CODEArguments *m_args;
  pqFilter      *m_filter;
  Segmentation  *m_seg;
};

class CODEFilter::CODEArguments
: public Arguments
{
public:
  static const ModelItem::ArgumentId INPUT;
  static const ModelItem::ArgumentId OPERATION;
  static const ModelItem::ArgumentId RADIUS;

public:
  explicit CODEArguments(){}
  explicit CODEArguments(const Arguments args);

  void setInput(Segmentation * seg)
  {
    (*this)[INPUT] = seg->volume().id();
  }

  void setOperation(Operation op)
  {
    m_operation = op;
    (*this)[OPERATION] = QString::number(op);
  }
  Operation operation() const {return m_operation;}

  void setRadius(unsigned int radius)
  {
    m_radius = radius;
    (*this)[OPERATION] = QString::number(radius);
  }
  unsigned int radius() const {return m_radius;}

private:
  Operation    m_operation;
  unsigned int m_radius;
};

#endif // CODEFILTER_H
