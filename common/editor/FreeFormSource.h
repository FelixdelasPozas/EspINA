/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef FREEFORMSOURCE_H
#define FREEFORMSOURCE_H

#include <model/Filter.h>
#include <common/views/vtkPVSliceView.h>

#include <QVector3D>

static const QString FFS = "EditorToolBar::FreeFormSource";

class FreeFormSource
: public Filter
{
public:
  explicit FreeFormSource(double spacing[3]);
  explicit FreeFormSource(Arguments args);
  virtual ~FreeFormSource();

  void draw(vtkPVSliceView::VIEW_PLANE plane,  QVector3D center, int radius = 0);
  void erase(vtkPVSliceView::VIEW_PLANE plane, QVector3D center, int radius = 0);
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
  Arguments     m_args;
  pqFilter     *m_source;
  Segmentation *m_seg;
  unsigned int  m_id;
  bool          m_hasPixels;
  static unsigned int m_count;
};

#endif // FREEFORMSOURCE_H
