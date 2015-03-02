/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_DEFAULT_VIEW_H
#define ESPINA_DEFAULT_VIEW_H

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <Support/ViewManager.h>
#include <Support/Settings/SettingsPanel.h>
#include <Support/Representations/RepresentationFactory.h>

// Qt
#include <QAbstractItemView>

class QMainWindow;
class QDockWidget;
class QUndoStack;

namespace ESPINA
{
  class RenderersMenu;
  class CamerasMenu;

  class DefaultView
  : QWidget
  {
    Q_OBJECT
  public:
    /** \brief DefaultView class constructor.
     * \param[in] model to be displayed by the view
     * \param[in] viewManager to coordinate render views
     * \param[in] undoStack ??
     * \param[in] parent of the qobject
     */
    explicit DefaultView(ModelAdapterSPtr    model,
                         ViewManagerSPtr     viewManager,
                         QUndoStack         *undoStack,
                         QMainWindow        *parent = nullptr);

    /** \brief DefaultView class virtual destructor.
     *
     */
    virtual ~DefaultView();

    void addRepresentation(const Representation &representation);

    /** \brief Sets the crosshair colors of the view.
     * \param[in] plane of the crosshair line.
     * \param[in] color of the crosshair line.
     *
     */
    void setCrosshairColor(const Plane plane, const QColor& color);

    /** \brief Fill the view menu.
     * \param[inout] menu to modify.
     *
     */
    void createViewMenu(QMenu *menu);

    /** \brief Returns the view's settings panel.
     *
     */
    SettingsPanelSPtr settingsPanel();

    /** \brief Loads view settings from storage.
     * \param[in] storage temporal storage containing the settings file.
     */
    void loadSessionSettings(TemporalStorageSPtr storage);

    /** \brief Saves view settings from storage.
     * \param[in] storage temporal storage to save settings file.
     */
    void saveSessionSettings(TemporalStorageSPtr storage);

  private slots:
    /** \brief Shows/hides the view's ruler.
     * \param[in] visible, true to show the ruler, false to hide.
     */
    void setRulerVisibility(bool visible);

    /** \brief Shows/hides the thumbnail in 2D views.
     * \param[in] visible, true to show the thumbnail, false to hide.
     */
    void showThumbnail(bool visible);

    /** \brief Toggles "fit to slices" boolean value.
     * \param[in] unused unused value.
     */
    void setFitToSlices(bool unused);

  private:
    void initView(RenderView *view);

    void deleteView(RenderView *view);

    void addRepresentationManager(RepresentationManagerSPtr manager);

    void addRepresentationPool(RepresentationPoolSPtr pool, const QString &group);

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;

    PipelineSources m_channelSources;
    PipelineSources m_segmentationSources;

    ViewStateSPtr              m_viewState;
    RepresentationPoolSList    m_channelPools;
    RepresentationPoolSList    m_segmentationPools;
    RepresentationPoolSList    m_autonomousPools;
    RepresentationManagerSList m_repManagers;

    bool m_showProcessing;

    NmVector3 m_slicingStep;

    QColor m_xLine, m_yLine, m_zLine;

    View2D *m_viewXY, *m_viewYZ, *m_viewXZ;
    View3D *m_view3D;

    QList<RenderView *> m_views;

    QDockWidget *dock3D, *dockYZ, *dockXZ;
    QAction     *m_showRuler, *m_showThumbnail;

    RenderersMenu      *m_renderersMenu;
    CamerasMenu        *m_camerasMenu;
  };


  using DefaultViewSPtr = std::shared_ptr<DefaultView>;

} // namespace ESPINA

#endif // ESPINA_DEFAULT_VIEW_H
