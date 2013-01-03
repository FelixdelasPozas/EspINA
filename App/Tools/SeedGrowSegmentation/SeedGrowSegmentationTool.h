/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SEEDGROWSEGMENTATIONTOOL_H
#define SEEDGROWSEGMENTATIONTOOL_H

#include <GUI/Tools/ITool.h>
#include <GUI/Pickers/IPicker.h>

#include <Core/Model/EspinaModel.h>

#include <QUndoCommand>

#include <vtkSmartPointer.h>

#include <itkConnectedThresholdImageFilter.h>
#include <itkImageToVTKImageFilter.h>

class DefaultVOIAction;
class ThresholdAction;
class vtkImageActor;
class vtkImageMapToColors;

namespace EspINA
{
  class SeedGrowSegmentationSettings;
  class SeedGrowSegmentationFilter;
  class ViewManager;

  class SeedGrowSegmentationTool
  : public ITool
  {
    Q_OBJECT
  public:
    explicit SeedGrowSegmentationTool(EspinaModel *model,
                                      QUndoStack  *undoStack,
                                      ViewManager *viewManager,
                                      ThresholdAction  *th,
                                      DefaultVOIAction *voi,
                                      SeedGrowSegmentationSettings *settings,
                                      IPickerSPtr picker = IPickerSPtr());

    virtual QCursor cursor() const;
    virtual bool filterEvent(QEvent* e, EspinaRenderView *view = 0);
    virtual void lostEvent(EspinaRenderView*);
    virtual bool enabled() const {return m_enabled;}
    virtual void setEnabled(bool enable);
    virtual void setInUse(bool value);

    void setChannelPicker(IPickerSPtr picker);

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
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    SeedGrowSegmentationSettings *m_settings;
    DefaultVOIAction *m_defaultVOI;
    ThresholdAction  *m_threshold;
    IPickerSPtr      m_picker;

    bool m_inUse;
    bool m_enabled;
    bool m_validPos;

    ConnectedThresholdFilterType::Pointer connectFilter;
    itk2vtkFilterType::Pointer i2v;
    vtkSmartPointer<vtkImageActor> m_actor;
    EspinaRenderView *m_viewOfPreview;
  };

  typedef QSharedPointer<SeedGrowSegmentationTool> SeedGrowSegmentationToolSPtr;

} // namespace EspINA

#endif // SEEDGROWSEGMENTATIONTOOL_H
