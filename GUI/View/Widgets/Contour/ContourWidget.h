/*

    Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CONTOUR_WIDGET_H_
#define ESPINA_CONTOUR_WIDGET_H_

// ESPINA
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/Selectors/BrushSelector.h>

// VTK
#include <vtkSmartPointer.h>

// Qt
#include <QMap>
#include <QList>
#include <QObject>
#include <QColor>

class vtkPolyData;
class vtkRenderWindowInteractor;
class vtkAbstractWidget;

namespace ESPINA
{
  class RenderView;
  class vtkPlaneContourWidget;

  class EspinaGUI_EXPORT ContourWidget
  : public QObject
  , public EspinaWidget
  {
  Q_OBJECT
  public:
    struct ContourInternals
    {
      Nm                       actualPosition;
      Nm                       contourPosition;
      Plane                    plane;
      BrushSelector::BrushMode mode;
      vtkPolyData             *polyData;

      ContourInternals(Nm actual, Nm position, Plane plane, BrushSelector::BrushMode mode, vtkPolyData *contour) : actualPosition{actual}, contourPosition{position}, plane{plane}, mode{mode}, polyData{contour} {};
      ContourInternals() : actualPosition{0}, contourPosition{0}, plane{Plane::XY}, mode{BrushSelector::BrushMode::BRUSH}, polyData{nullptr} {};
    };

    using ContourData = struct ContourInternals;
    using ContourList = QList<ContourData>;

    /** \brief ContourWidget class constructor.
     *
     */
    explicit ContourWidget();

    /** \brief ContourWidget class virtual destructor.
     *
     */
    virtual ~ContourWidget();

    virtual void registerView  (RenderView *view);
    virtual void unregisterView(RenderView *view);

    virtual void setEnabled(bool enable);

    /** \brief Changes the polygon of the area inside the contour to the given color.
     * \param[in] color QColor object.
     *
     */
    virtual void setPolygonColor(QColor color);

    /** \brief Returs the actual color of the polygon defined by the contour.
     *
     */
    virtual QColor getPolygonColor();

    /** \brief Returns the list of defined contours.
     *
     */
    ContourList getContours();

    /** \brief Starts the interaction.
     *
     */
    void startContourFromWidget();

    /** \brief Ends the widget interaction.
     *
     */
    void endContourFromWidget();

    /** \brief Sets the mode of the widget.
     * \param[in] mode Brush mode.
     */
    void setMode(BrushSelector::BrushMode mode);

    // reset all contours in all planes without rasterize
    /** \brief Resets all contours in all planes without rasterizing.
     *
     */
    void initialize();

    /** \brief Resets all contours in all planes without rasterizing and uses the given contour list as initial data.
     * \param[in] contour initial contour.
     *
     */
    void initialize(ContourData contour);

  signals:
    void rasterizeContours(ContourWidget::ContourList);
    void endContour();

  protected slots:
    /** \brief Stored the finished contours if necessary and manages the change of slices in the widgets.
     * \param[in] plane orientation of the change of slice.
     * \param[in] pos new slice position.
     *
     */
    void changeSlice(Plane plane, Nm pos);

  private:
    friend class ContourSelector;

    QMap<View2D*, vtkSmartPointer<vtkPlaneContourWidget>> m_widgets;
    QMap<View2D*, ContourData> m_storedContours;

    QColor m_color;
  };

  using ContourWidgetPtr  = ContourWidget *;
  using ContourWidgetSPtr = std::shared_ptr<ContourWidget>;

} // namespace ESPINA;

#endif // ESPINA_CONTOUR_WIDGET_H_
