/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_VIEW_STATE_H
#define ESPINA_VIEW_STATE_H

// ESPINA
#include <Core/Utils/NmVector3.h>
#include <Core/Utils/Bounds.h>
#include <Core/EspinaTypes.h>
#include <GUI/Utils/Timer.h>
#include "CoordinateSystem.h"
#include "EventHandler.h"
#include "Selection.h"
#include "Widgets/EspinaWidget.h"

// Qt
#include <QObject>

namespace ESPINA
{
  class ViewState
  : public QObject
  {
    Q_OBJECT

  public:
    /** \brief Class ViewState class constructor.
     * \param[in] timer state timer object.
     *
     */
    explicit ViewState(GUI::View::CoordinateSystemSPtr coordinateSystem = std::make_shared<GUI::View::CoordinateSystem>(),
                       TimerSPtr timer = std::make_shared<Timer>());

    TimeStamp timeStamp() const;

    NmVector3 crosshair() const;

    /** \brief Enables/disables the "fit to slices" flag.
     * \param[in] value true to enable false otherwise.
     *
     * If fit to slices is enabled the movement between slices is the resolution of the scene.
     *
     */
    void setFitToSlices(bool value);

    /** \brief Returns the value of the "fit to slices" flag.
     *
     */
    bool fitToSlices() const;

    /** \brief Sets the view event handler.
     * \param[in] handler active event handler
     *
     */
    void setEventHandler(EventHandlerSPtr handler);

    /** \brief Sets the view event handler.
     * \param[in] handler event handler to be deactivated
     *
     */
    void unsetEventHandler(EventHandlerSPtr handler);

    /** \brief Returns current event handler
     *
     */
    EventHandlerSPtr eventHandler() const;

    SelectionSPtr selection();

    /** \brief Adds a widget to the view.
     * \param[in] widget espina widget smart pointer.
     *
     */
    virtual void addWidget(EspinaWidgetSPtr widget);

    /** \brief Removes a widget to the view.
     * \param[in] widget espina widget smart pointer.
     *
     */
    virtual void removeWidget(EspinaWidgetSPtr widget);

    GUI::View::CoordinateSystemSPtr coordinateSystem() const;

    /** \brief Ensure point position is visible
     *
     */
    void focusViewOn(const NmVector3 &point);

    void resetCamera();

    void refresh();

  public slots:
    /** \brief Changes the crosshair position to point
     *
     */
    void setCrosshair(const NmVector3 &point);

    /** \brief Changes the crosshair position of the given plane
     *
     */
    void setCrosshairPlane(const Plane plane, const Nm position);

  signals:
    void crosshairChanged(const NmVector3 &point, TimeStamp t);

    void sceneResolutionChanged(const NmVector3 &resolution, TimeStamp t);

    void sceneBoundsChanged(const Bounds &bounds, TimeStamp t);

    void viewFocusedOn(NmVector3);

    void resetCameraRequested();

    void refreshRequested();

  private:
    NmVector3 crosshairPoint(const NmVector3 &point) const;

    NmVector3 voxelCenter(const NmVector3 &point) const;

    void changeCrosshair(const NmVector3 &point);

  private slots:
    void onResolutionChanged(const NmVector3 &resolution);

    void onBoundsChanged(const Bounds &bounds);

  private:
    TimerSPtr m_timer;
    NmVector3 m_crosshair;
    bool      m_fitToSlices;
    GUI::View::CoordinateSystemSPtr m_coordinateSystem;
    SelectionSPtr    m_selection;
    EventHandlerSPtr m_eventHandler;
  };

  using ViewStateSPtr = std::shared_ptr<ViewState>;

  void updateSceneState(ViewStateSPtr state, ViewItemAdapterSList viewItems);
}

#endif // ESPINA_VIEWSTATE_H
