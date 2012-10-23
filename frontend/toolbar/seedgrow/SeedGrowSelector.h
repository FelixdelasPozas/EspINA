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

#include "common/tools/IPicker.h"
#include "common/tools/ITool.h"
#include <EspinaTypes.h>

class Channel;
class SeedGrowSegmentationFilter;
class ThresholdAction;
class ViewManager;

class SeedGrowSelector
: public ITool
{
  Q_OBJECT
public:
  explicit SeedGrowSelector(ThresholdAction *th,
                            ViewManager *vm,
                            IPicker *picker = NULL);

  virtual QCursor cursor() const;
  virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);
  virtual void setInUse(bool enable);
  virtual void setEnabled(bool enable);
  virtual bool enabled() const {return m_interactive;}

  void previewOn();
  void previewOff();

  void setChannelPicker(IPicker *picker);

public slots:
  void extractSeed(IPicker::PickList pickedItems);
  void abortSelection(){}

signals:
  void seedSelected(Channel *, EspinaVolume::IndexType);
  void selectionAborted();

private:
  ThresholdAction *m_threshold;
  ViewManager *m_viewManager;
  IPicker *m_picker;

  bool m_enabled;
  bool m_interactive;
  SeedGrowSegmentationFilter *m_preview;
};

#endif // SEEDGROWSELECTOR_H
