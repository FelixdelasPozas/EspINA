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
#ifndef ESPINA_VIEW_MANAGER_H
#define ESPINA_VIEW_MANAGER_H

#include "Support/EspinaSupport_Export.h"

// EspINA
#include <Core/Utils/Measure.h>
#include <GUI/View/SelectableView.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Selectors/Selector.h>

// Qt
#include <QList>
#include <QMap>
#include <QColor>

// VTK
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class QToolBar;
class QAction;
class QCursor;
class QEvent;

namespace EspINA
{
  class SliceSelectorWidget;
  class RenderView;
  class SelectableView;
  class Selector;
  class View2D;
  class Measure;

  class ToolGroup;
  using ToolGroupPtr = ToolGroup*;

  using ROI = Selector::SelectionMaskSPtr;

  class EspinaSupport_EXPORT ViewManager
  : public QObject
  {
    Q_OBJECT
  public:
    explicit ViewManager();
    ~ViewManager();

    void registerView(SelectableView* view);
    void registerView(RenderView*     view);
    void registerView(View2D*         view);

    void unregisterView(SelectableView* view);
    void unregisterView(RenderView*     view);
    void unregisterView(View2D*         view);

  private:
    QList<SelectableView *> m_espinaViews;
    QList<RenderView *>     m_renderViews;
    QList<View2D *>         m_sliceViews;

    //---------------------------------------------------------------------------
    /*************************** Selection API *********************************/
    //---------------------------------------------------------------------------
  public:
    /** \brief Enable item selection in views controlled by the ViewManager
     * 
     */
    void setSelectionEnabled(bool enable);

    /** \brief Request all controlled views to update their selection
     *
     */
    void setSelection(ViewItemAdapterList selection);

    /** \brief Return selection shared amongs all the views controlled by the ViewManager
     *
     */
    SelectionSPtr selection() const
    { return m_selection; }

  private:
    SelectionSPtr m_selection;

    //---------------------------------------------------------------------------
    /************************* Tool Group API ********************************/
    //---------------------------------------------------------------------------
  public:
    void setContextualBar(QToolBar *toolbar)
    { m_contextualToolBar = toolbar; }

    void displayTools(ToolGroupPtr group);

    void hideTools(ToolGroupPtr group);

//     ToolGroupPtr toolGroup()
//     { return m_toolGroup; }
    void setSelector(SelectorSPtr selector);

    void unsetSelector(SelectorSPtr selector);

    void setCurrentROI(ROI roi)
    { m_roi = roi; }

    ROI currentROI() const
    { return m_roi; }

  private:
    ROI          m_roi;
    QToolBar    *m_contextualToolBar;
    ToolGroupPtr m_toolGroup;
    SelectorSPtr m_selector;

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
    void focusViewsOn(const NmVector3& point);

    /// Toggle segmentations visibility
    void setSegmentationVisibility(bool visible);

    /// Toggle crosshair visibility
    void setCrosshairVisibility(bool visible);

//     /// Set Slice Selection flags to all registered Slice Views
//     void addSliceSelectors(SliceSelectorWidget *widget,
//                            ViewManager::SliceSelectors selectors);
//     /// Unset Slice Selection flags to all registered Slice Views
//     void removeSliceSelectors(SliceSelectorWidget *widget);

    QAction *fitToSlices()
    {return m_fitToSlices;}

    NmVector3 viewResolution();

    MeasureSPtr measure(Nm distance);

  public slots:
    /// Update Segmentation Representation
    void updateSegmentationRepresentations(SegmentationAdapterPtr segmentation);

    /// Update Segmentation Representation
    void updateSegmentationRepresentations(SegmentationAdapterList list = SegmentationAdapterList());

    /// Update Channel Representation
    void updateChannelRepresentations(ChannelAdapterList list = ChannelAdapterList());

    /// Request all registered views to update themselves
    void updateViews();

    /// Change fit to slice settings
    void setFitToSlices(bool enabled);

private:
  QAction*  m_fitToSlices;
  NmVector3 m_viewResolution;
  QString   m_resolutionUnits;

    //---------------------------------------------------------------------------
    /*********************** Active Elements API *******************************/
    //---------------------------------------------------------------------------
    // These are specified by the user to be used when one element of the      //
    // proper type is required                                                 //
    //---------------------------------------------------------------------------
  public:
    void setActiveChannel(ChannelAdapterPtr channel);

    ChannelAdapterPtr activeChannel() const
    { return m_activeChannel; }

    void setActiveCategory(CategoryAdapterPtr category)
    { m_activeCategory = category; }

    CategoryAdapterPtr activeCategory() const
    { return m_activeCategory; }

  signals:
    void activeChannelChanged(ChannelAdapterPtr);
    void activeCategoryChanged(CategoryAdapterPtr);

  private:
    ChannelAdapterPtr  m_activeChannel;
    CategoryAdapterPtr m_activeCategory;

    //---------------------------------------------------------------------------
    /************************* Color Engine API ********************************/
    //---------------------------------------------------------------------------
  public:
    void setColorEngine(ColorEngineSPtr engine);

  private:
    ColorEngineSPtr m_colorEngine;
  };

  using ViewManagerSPtr = std::shared_ptr<ViewManager>;

//   Q_DECLARE_OPERATORS_FOR_FLAGS(ViewManager::SliceSelectors)
}

#endif // ESPINA_VIEW_MANAGER_H
