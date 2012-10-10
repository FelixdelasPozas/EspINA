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


#ifndef SEEDGROWSELECTOR_H
#define SEEDGROWSELECTOR_H

#include "common/selection/IPicker.h"

class SeedGrowSegmentationFilter;
class ThresholdAction;

class SeedGrowSelector
: public IPicker
{
  Q_OBJECT
public:
  explicit SeedGrowSelector(ThresholdAction *th, IPicker *succesor = NULL);

  virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);
  virtual QCursor cursor();

  void previewOn();
  void previewOff();

  void setPixelSelector(IPicker *sel) {m_succesor = sel;}

private:
  ThresholdAction *m_threshold;
  SeedGrowSegmentationFilter *m_preview;
};

#endif // SEEDGROWSELECTOR_H
