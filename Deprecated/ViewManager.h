/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_VIEW_MANAGER_H
#define ESPINA_VIEW_MANAGER_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Core/Utils/Measure.h>
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <GUI/View/SelectableView.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/View/Selection.h>
#include <GUI/View/EventHandler.h>
#include <GUI/View/View2D.h>
#include <GUI/Widgets/SliceSelector.h>
#include <GUI/Utils/Timer.h>
#include "ROIProvider.h"

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

namespace ESPINA
{
  class SelectableView;
  class Measure;

  class ToolGroup;
  using ToolGroupPtr = ToolGroup*;

  /** \class ViewManager.
   * \brief Singleton to register and keep views updated.
   *
   */
  class EspinaSupport_EXPORT ViewManager
  : public QObject
  {
    Q_OBJECT
  public:
    /** \brief ViewManager class constructor.
     *
     */
    explicit ViewManager();

    /** \brief ViewManager class destructor.
     *
     */
    ~ViewManager();

    //---------------------------------------------------------------------------
    /*********************** Configuration API *********************************/
    // Configure the view manager components
    //---------------------------------------------------------------------------
    /** \brief Returns a list of raw pointers of registered render views (2D and 3D views).
     *
     */
    QList<RenderView*> renderViews()
    { return m_renderViews; }

    /** \brief Returns a list of raw pointers of registered selectable views.
     *
     */
    QList<SelectableView*> selectableViews()
    { return m_espinaViews; }

    /** \brief Returns a list of raw pointers of registered 2D views.
     *
     */
    QList<View2D *> sliceViews();

  private:
    QList<SelectableView *> m_espinaViews;
    QList<RenderView *>     m_renderViews;

  public:
    /** \brief Removes all view items from the registered views
     *
     */
    void removeAllViewItems();

    virtual void updateRepresentations(ChannelAdapterList list){}

    virtual void updateRepresentations(SegmentationAdapterList list){}

    virtual void updateRepresentations(){}

  public slots:
    /** \brief Request all registered views to update themselves.
     *
     */
    void updateViews(); // NOTE: It would be interesting to deprecate it

    /** \brief Change "fit to slice" flag.
     * \param[in] enabled true to enable "fit to slices" false otherwise.
     *
     */
    void setFitToSlices(bool enabled);

    //---------------------------------------------------------------------------
    /*************************** Selection API *********************************/
    //---------------------------------------------------------------------------
  public:
    /** \brief Enable item selection in views controlled by the ViewManager.
     * \param[in] enable true to enable the user to select objects in the view, false otherwise.
     *
     */
    void setSelectionEnabled(bool enable);

    /** \brief Request all controlled views to update their selection.
     * \param[in] selection current selection.
     *
     */
    void setSelection(ViewItemAdapterList selection);

    /** \brief Return selection shared among all the views controlled by the ViewManager
     *
     */
    SelectionSPtr selection() const
    { return m_selection; }

  signals:
    void selectionChanged(SelectionSPtr);

  private:
    SelectionSPtr m_selection;

    //---------------------------------------------------------------------------
    /************************* Tool Group API ********************************/
    //---------------------------------------------------------------------------
  public:
    /** \brief Sets the event handler as the active one.
     * \param[in] eventHandler event handler smart pointer.
     *
     */
    void setEventHandler(EventHandlerSPtr eventHandler);

    /** \brief Unsets the active event handler.
     *
     */
    void unsetActiveEventHandler();

    /** \brief Unsets the specified eventhandler if it was enabled.
     * \param[in] eventHandler event handler smart pointer.
     *
     */
    void unsetEventHandler(EventHandlerSPtr eventHandler);

    /** \brief Returns the current event handler.
     *
     */
    EventHandlerSPtr eventHandler() const
    { return m_eventHandler; }

    /** \brief Sets the current region of interest provider
     * \param[in] provider ROI Provider smart pointer.
     *
     */
    void setROIProvider(ROIProviderPtr provider)
    { m_roiProvider = provider;}

    /** \brief Returns the current region of interest provider.
     *
     */
    ROIProviderPtr roiProvider() const
    { return m_roiProvider; }

    ROISPtr currentROI() const
    { return m_roiProvider?m_roiProvider->currentROI():ROISPtr();}

    void consumeROI()
    { if (m_roiProvider) m_roiProvider->consumeROI();}

  signals:
    void roiRoviderChanged();
    void eventHandlerChanged();

  private:
    ROIProviderPtr   m_roiProvider;
    EventHandlerSPtr m_eventHandler;

    //---------------------------------------------------------------------------
    /***************************** Widget API **********************************/
    //---------------------------------------------------------------------------
  public:
    /** \brief Adds a widget to the registered views.
     * \param[in] widget smart pointer of the widget to add.
     *
     */
    void addWidget(EspinaWidgetSPtr widget);

    /** \brief Removes a widget from the registered views.
     * \param[in] widget smart pointer of the widget to remove.
     *
     */
    void removeWidget(EspinaWidgetSPtr widget);

  private:
    QList<EspinaWidgetSPtr> m_widgets;

    //---------------------------------------------------------------------------
    /*********************** View Synchronization API **************************/
    //---------------------------------------------------------------------------
  public:
    /** \brief Resets the camera of all the views.
     *
     */
    void resetViewCameras();

    /** \brief Centers the views on the specified point.
     * \param[in] point point to center the views on.
     *
     */
    void focusViewsOn(const NmVector3& point);

    /** \brief
     *
     */
    void addSliceSelectors(SliceSelectorSPtr widget,
                           View2D::SliceSelectionType selectors);
    /// Unset Slice Selection flags to all registered Slice Views
    void removeSliceSelectors(SliceSelectorSPtr widget);

    /** \brief Returns the QAction of the "fit to slices" action.
     *
     */
    QAction *fitToSlices()
    {return m_fitToSlices;}

    /** \brief Returns the resolution of the view in every axis.
     *
     */
    NmVector3 viewResolution();

    /** \brief Returns a measure object of the provided measure in Nm.
     * \param[in] distance measure in Nm units.
     *
     */
    MeasureSPtr measure(Nm distance);


private:
  QAction*  m_fitToSlices;
  NmVector3 m_viewResolution;
  QString   m_resolutionUnits;
  GUI::View::ViewState & m_viewState;

    //---------------------------------------------------------------------------
    /*********************** Active Elements API *******************************/
    //---------------------------------------------------------------------------
    // These are specified by the user to be used when one element of the      //
    // proper type is required                                                 //
    //---------------------------------------------------------------------------
  public:
    /** \brief Sets the active channel for the views.
     * \param[in] channel raw pointer of the channel adapter.
     *
     */
    void setActiveChannel(ChannelAdapterPtr channel);

    /** \brief Returns a raw pointer of the active channel.
     *
     */
    ChannelAdapterPtr activeChannel() const
    { return m_activeChannel; }

    /** \brief Sets the active category.
     * \param[in] category raw pointer of the new active category.
     *
     */
    void setActiveCategory(CategoryAdapterPtr category)
    { m_activeCategory = category; }

    /** \brief Returns a raw pointer of the active category.
     *
     */
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
    /** \brief Sets the color engine for the views.
     * \param[in] engine, color engine smart pointer.
     *
     */
    void setColorEngine(ColorEngineSPtr engine);

    /** \brief Returns the color engine smart pointer.
     *
     */
    ColorEngineSPtr colorEngine() const
    { return m_colorEngine;}

  private:
    ColorEngineSPtr m_colorEngine;
  };

  using ViewManagerSPtr = std::shared_ptr<ViewManager>;
}

#endif // ESPINA_VIEW_MANAGER_H
