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


#ifndef SEEDGROWSEGMENTATIONTOOL_H
#define SEEDGROWSEGMENTATIONTOOL_H

#include <common/tools/ITool.h>

#include "SeedGrowSegmentation.h"

#include <common/tools/IPicker.h>

#include <QUndoCommand>

class DefaultVOIAction;
class ThresholdAction;
class Channel;
class Sample;
class Segmentation;
class SeedGrowSegmentationFilter;
class TaxonomyElement;

class SeedGrowSegmentationTool
: public ITool
{
  class CreateSegmentation
  : public QUndoCommand
  {
  public:
    explicit CreateSegmentation(Channel * channel,
                                SeedGrowSegmentationFilter *filter,
                                TaxonomyElement *taxonomy,
                                EspinaModel *model);
    virtual void redo();
    virtual void undo();
  private:
    EspinaModel                *m_model;
    Sample                     *m_sample;
    Channel                    *m_channel;
    SeedGrowSegmentationFilter *m_filter;
    Segmentation               *m_seg;
    TaxonomyElement            *m_taxonomy;
  };

  Q_OBJECT
public:
  explicit SeedGrowSegmentationTool(EspinaModel *model,
                                    QUndoStack  *undoStack,
                                    ViewManager *viewManager,
                                    ThresholdAction  *th,
                                    DefaultVOIAction *voi,
                                    SeedGrowSegmentation::Settings *settings,
                                    IPicker *picker = NULL);

  virtual QCursor cursor() const;
  virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);
  virtual bool enabled() const {return m_enabled;}
  virtual void setEnabled(bool enable);
  virtual void setInUse(bool value);

  void setChannelPicker(IPicker *picker);

public slots:
  void startSegmentation(IPicker::PickList pickedItems);

private:
  void previewOn();
  void previewOff();

signals:
  void seedSelected(Channel *, EspinaVolume::IndexType);
  void segmentationStopped();

private:
  EspinaModel *m_model;
  QUndoStack  *m_undoStack;
  ViewManager *m_viewManager;

  SeedGrowSegmentation::Settings *m_settings;
  DefaultVOIAction *m_defaultVOI;
  ThresholdAction  *m_threshold;
  IPicker *m_picker;

  bool m_inUse;
  bool m_enabled;
  SeedGrowSegmentationFilter *m_preview;
};

#endif // SEEDGROWSEGMENTATIONTOOL_H
