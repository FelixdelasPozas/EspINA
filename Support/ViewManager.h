/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
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


//----------------------------------------------------------------------------
// File:    ViewManager.h
// Purpose: Singleton to register and keep views updated
//----------------------------------------------------------------------------
#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include "EspinaGUI_Export.h"

// EspINA
#include "Core/EspinaTypes.h"
#include "Core/ColorEngines/IColorEngine.h"
#include "GUI/Tools/IVOI.h"
#include "GUI/vtkWidgets/EspinaWidget.h"
#include <Core/Utils/Measure.h>

// Qt
#include <QList>
#include <QMap>
#include <QColor>

// VTK
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class QAction;
class QCursor;
class QEvent;

namespace EspINA
{
  class SliceSelectorWidget;
  class RenderView;
  class SelectableView;
  class ISelector;
  class ITool;
  class IVOI;
  class SliceView;
  class Measure;

  class EspinaGUI_EXPORT ViewManager
  : public QObject
  {
    Q_OBJECT
  public:
    explicit ViewManager();
    ~ViewManager();

    void registerView(SelectableView *view);
    void registerView(RenderView *view);
    void registerView(SliceView *view);

    void unregisterView(SelectableView *view);
    void unregisterView(RenderView *view);
    void unregisterView(SliceView *view);

  private:
    QList<SelectableView *>      m_espinaViews;
    QList<RenderView *> m_renderViews;
    QList<SliceView *>        m_sliceViews;

    //---------------------------------------------------------------------------
    /*************************** Selection API *********************************/
    //---------------------------------------------------------------------------
  public:
    typedef QList<PickableItemPtr> Selection; // Ya esta definido en IPicker...

    /// Enable item selection in render views
    void setSelectionEnabled(bool enable);
    /// Synchronize @selection between all registered views
    void setSelection(Selection selection);
    /// Returns current selection
    Selection selection() const { return m_selection; }
    SegmentationList selectedSegmentations() const;
    //   const Nm *selectionCenter() const
    //   { return m_selectionCenter; }
    void clearSelection(bool notifyViews = true);

  signals:
    void selectionChanged(ViewManager::Selection, bool);

  private:
    Selection m_selection;

    //---------------------------------------------------------------------------
    /*************************** Picking API *********************************/
    //---------------------------------------------------------------------------
  public:
    void setVOI(IVOISPtr voi);
    void unsetActiveVOI();
    IVOISPtr voi() {return m_voi;}
    IVOI::Region voiRegion() {return m_voi?m_voi->region():NULL;}
    /// Set @tool as active tool. If other tool is already active,
    /// it will be disactivated
    void setActiveTool(IToolSPtr tool);
    /// Unset any active tool
    void unsetActiveTool();
    /// Unset @tool as active tool
    void unsetActiveTool(IToolSPtr tool);
    /// Filter @view's @event.
    /// Delegate active voi event handling. If the event is not filtered by
    /// active voi, then active tool, if any, filter the event. If it neither
    /// filter the event, the function returns false. Otherwise, returns true.
    bool filterEvent(QEvent *event, RenderView *view=NULL);
    QCursor cursor() const;

  private:
    IVOISPtr  m_voi;
    IToolSPtr m_tool;

    //---------------------------------------------------------------------------
    /***************************** Widget API **********************************/
    //---------------------------------------------------------------------------
  public:
    void addWidget   (EspinaWidget *widget);
    void removeWidget(EspinaWidget *widget);

    //---------------------------------------------------------------------------
    /*********************** View Synchronization API **************************/
    //---------------------------------------------------------------------------
  public:

    /// Reset Camera
    void resetViewCameras();
    /// Focus
    void focusViewsOn(Nm *);
    /// Toggle crosshair
    void showCrosshair(bool);
    /// Set Slice Selection flags to all registered Slice Views
    void addSliceSelectors(SliceSelectorWidget *widget,
                           ViewManager::SliceSelectors selectors);
    /// Unset Slice Selection flags to all registered Slice Views
    void removeSliceSelectors(SliceSelectorWidget *widget);

    QAction *fitToSlices() {return m_fitToSlices;}

    Nm *viewResolution();
    MeasureSPtr measure(Nm distance);

  public slots:
    /// Update Segmentation Representation
    void updateSegmentationRepresentations(SegmentationPtr segmentation);
    /// Update Segmentation Representation
    void updateSegmentationRepresentations(SegmentationList list = SegmentationList());
    /// Update Channel Representation
    void updateChannelRepresentations(ChannelList list = ChannelList());
    /// Request all registered views to update themselves
    void updateViews();
    /// Change fit to slice settings
    void toggleFitToSlices(bool);

private:
  QAction *m_fitToSlices;
  Nm m_viewResolution[3];
  QString m_resolutionUnits;

    //---------------------------------------------------------------------------
    /*********************** Active Elements API *******************************/
    //---------------------------------------------------------------------------
    // These are specified by the user to be used when one element of the      //
    // proper type is required                                                 //
    //---------------------------------------------------------------------------
  public:
    void setActiveChannel(ChannelPtr channel);
    ChannelPtr activeChannel() { return m_activeChannel; }
    void setActiveTaxonomy(TaxonomyElementPtr taxonomy) { m_activeTaxonomy = taxonomy; }
    TaxonomyElementPtr activeTaxonomy() { return m_activeTaxonomy; }

  signals:
    void activeChannelChanged(ChannelPtr);
    void activeTaxonomyChanged(TaxonomyElementPtr);

    //---------------------------------------------------------------------------
    /************************* Color Engine API ********************************/
    //---------------------------------------------------------------------------
  public:
    QColor color(SegmentationPtr seg);
    LUTPtr lut  (SegmentationPtr seg);

    void setColorEngine(ColorEngine *engine);

  private:
    ChannelPtr         m_activeChannel;
    TaxonomyElementPtr m_activeTaxonomy;

    ColorEngine *m_colorEngine;
    vtkSmartPointer<vtkLookupTable> seg_lut;
  };

  Q_DECLARE_OPERATORS_FOR_FLAGS(ViewManager::SliceSelectors)
}


#endif // VIEWMANAGER_H
