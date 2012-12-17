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

#include <GUI/Tools/ITool.h>
#include <GUI/Pickers/IPicker.h>
#include <Toolbars/SeedGrowSegmentation/SeedGrowSegmentation.h>

#include <QUndoCommand>

#include <vtkSmartPointer.h>

#include <itkConnectedThresholdImageFilter.h>
#include <itkImageToVTKImageFilter.h>

class vtkImageActor;
class vtkImageMapToColors;

namespace EspINA
{
  class SeedGrowSegmentationFilter;
  class ViewManager;

  class SeedGrowSegmentationTool
  : public ITool
  {
    class CreateSegmentation
    : public QUndoCommand
    {
    public:
      explicit CreateSegmentation(ChannelPtr channel,
                                  FilterPtr filter,
                                  SegmentationPtr segmentation,
                                  TaxonomyElementPtr taxonomy,
                                  EspinaModelPtr model);
      virtual void redo();
      virtual void undo();
    private:
      EspinaModelPtr     m_model;
      SamplePtr          m_sample;
      ChannelPtr         m_channel;
      FilterPtr          m_filter;
      SegmentationPtr    m_seg;
      TaxonomyElementPtr m_taxonomy;
    };

    Q_OBJECT
  public:
    explicit SeedGrowSegmentationTool(EspinaModelPtr model,
                                      QUndoStack  *undoStack,
                                      ViewManager *viewManager,
                                      ThresholdAction  *th,
                                      DefaultVOIAction *voi,
                                      SeedGrowSegmentation::Settings *settings,
                                      IPicker *picker = NULL);

    virtual QCursor cursor() const;
    virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);
    virtual void lostEvent(EspinaRenderView*);
    virtual bool enabled() const {return m_enabled;}
    virtual void setEnabled(bool enable);
    virtual void setInUse(bool value);

    void setChannelPicker(IPicker *picker);

  public slots:
    void startSegmentation(IPicker::PickList pickedItems);

  signals:
    void seedSelected(Channel *, itkVolumeType::IndexType);
    void segmentationStopped();

  private:
    typedef itk::ConnectedThresholdImageFilter<itkVolumeType, itkVolumeType> ConnectedThresholdFilterType;
    typedef itk::ImageToVTKImageFilter<itkVolumeType> itk2vtkFilterType;

    // helper methods
    void removePreview(EspinaRenderView*);
    void addPreview(EspinaRenderView*);

  private:
    EspinaModelPtr m_model;
    QUndoStack    *m_undoStack;
    ViewManager   *m_viewManager;

    SeedGrowSegmentation::Settings *m_settings;
    DefaultVOIAction *m_defaultVOI;
    ThresholdAction  *m_threshold;
    IPicker *m_picker;

    bool m_inUse;
    bool m_enabled;
    bool m_validPos;

    ConnectedThresholdFilterType::Pointer connectFilter;
    itk2vtkFilterType::Pointer i2v;
    vtkSmartPointer<vtkImageActor> m_actor;
    EspinaRenderView *m_viewOfPreview;
  };

} // namespace EspINA

#endif // SEEDGROWSEGMENTATIONTOOL_H
