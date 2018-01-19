/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_VIEW_3D_H
#define ESPINA_VIEW_3D_H

#include <GUI/View/RenderView.h>

// ESPINA
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkCommand.h>
#include <vtkCamera.h>

// Qt
#include <QPushButton>
#include <QDoubleSpinBox>

class vtkAbstractWidget;
class QVTKWidget;

//Forward declaration
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QScrollBar;
class QLabel;

namespace ESPINA
{
  class EspinaGUI_EXPORT View3D
  : public RenderView
  {
    Q_OBJECT
  public:
    /** \brief View3D class constructor.
     * \param[in] showCrosshairPlaneSelectors true to show three aditional
     *            scrollbars in the borders of the view to manipulate the
     *            crosshair point, false otherwise.
     * \param[in] parent raw pointer of the QWidget parent of this one.
     *
     */
    explicit View3D(GUI::View::ViewState &state, bool showCrosshairPlaneSelectors = false, QWidget *parent = nullptr);

    /** \brief View3D class virtual destructor.
     *
     */
    virtual ~View3D();

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const override;

    virtual bool eventFilter(QObject* caller, QEvent* e) override;

    virtual void setCameraState(CameraState state) override;

    virtual RenderView::CameraState cameraState() override;

  protected:
    virtual void resetImplementation() override;

  protected slots:
    /** \brief Updates the view then a crosshair scroll bar changes value.
     * \param[in] value new scrollbar value.
     *
     */
    void scrollBarMoved(int value);

    /** \brief Exports the scene meshes to an external format and saves the result to disk.
     *
     */
    void exportScene();

    /** \brief Takes an 2D image of the current view and saves it to disk.
     *
     */
    void onTakeSnapshot();

    /** \brief Updates the camera if the user changes the focal distance.
     * \param[in] distance new distance value.
     *
     */
    void onFocalDistanceChanged(double distance);

  private:
    virtual void onCrosshairChanged(const GUI::Representations::FrameCSPtr frame) override;

    virtual void moveCamera(const NmVector3 &point) override;

    virtual void onSceneResolutionChanged(const NmVector3 &reslotuion) override;

    virtual void onSceneBoundsChanged(const Bounds &bounds) override;

    virtual Selector::Selection pickImplementation(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const override;

    virtual void addActor   (vtkProp *actor) override;

    virtual void removeActor(vtkProp *actor) override;

    virtual vtkRenderer *mainRenderer() const override;

    virtual void updateViewActions(GUI::Representations::RepresentationManager::ManagerFlags flags) override;

    virtual void resetCameraImplementation() override;

    virtual bool isCrosshairPointVisible() const override;

    virtual void refreshViewImplementation() override;

    /** \brief Helper method to setup the UI.
     *
     */
    void setupUI();

    /** \brief Helper method to build the UI controls.
     *
     */
    void buildViewActionsButtons();

    /** \brief Helper method to update the limit of the crosshair scrollbars.
     *
     */
    void updateScrollBarsLimits();

    virtual const QString viewName() const override;

    /** \brief Connects the camera signal to this object.
     *
     */
    void connectCamera();

  private:
    /** \class vtkCameraCommand
     * \brief Callback for camera modification event.
     *
     */
    class vtkCameraCommand
    : public vtkCommand
    {
      public:
        /** \brief vtkCameraCommand class vtk-style new() operator.
         *
         */
        static vtkCameraCommand *New()
        { return new vtkCameraCommand; }

        virtual void Execute(vtkObject *caller, unsigned long, void*)
        {
          auto camera = vtkCamera::SafeDownCast(caller);

          if(camera && m_view)
          {
            if(std::abs(camera->GetDistance() - m_view->m_zoomFactor->value()) < 0.01) return;

            m_view->m_zoomFactor->setValue(camera->GetDistance());
          }
        }

        /** \brief Sets the view the command with interact with.
         * \param[in] view view pointer.
         *
         */
        void SetView(View3D *view)
        { m_view = view; }

      protected:
        /** \brief vtkCameraCommand class protected constructor.
         *
         */
        explicit vtkCameraCommand()
        : m_view{nullptr}
        {};

      private:
        View3D* m_view; /** Qt view with the camera. */
    };

    friend class vtkCameraCommand;

    // GUI
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QPushButton *m_snapshot;
    QPushButton *m_export;
    QPushButton *m_cameraReset;
    QPushButton *m_renderConfig;

    QDoubleSpinBox                   *m_zoomFactor;
    vtkSmartPointer<vtkCameraCommand> m_cameraCommand;

    // GUI elements only visible in Segmentation Information dialog
    QHBoxLayout *m_additionalGUI;
    QScrollBar  *m_axialScrollBar;
    QScrollBar  *m_coronalScrollBar;
    QScrollBar  *m_sagittalScrollBar;

    vtkSmartPointer<vtkRenderer> m_renderer;
    bool m_showCrosshairPlaneSelectors;
  };

  /** \brief Returns the 3D view raw pointer given a RenderView raw pointer.
   * \param[in] view, RenderView raw pointer.
   *
   */
  inline View3D * view3D_cast(RenderView* view)
  { return dynamic_cast<View3D *>(view); }

  /** \brief Returns true if the view is a 3D view.
   * \param[in] view RenderView raw pointer.
   *
   */
  inline bool isView3D(RenderView *view)
  { return view3D_cast(view) != nullptr; }

  using View3DSPtr = std::shared_ptr<View3D>;

} // namespace ESPINA

#endif // ESPINA_VIEW_3D_H
