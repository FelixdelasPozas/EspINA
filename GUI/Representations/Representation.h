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

#ifndef ESPINA_REPRESENTATION_H
#define ESPINA_REPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QList>
#include <QString>
#include <QColor>

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/NmVector3.h>

class vtkProp3D;
class vtkProp;
class vtkMatrix4x4;
class vtkLookupTable;

namespace ESPINA
{
  class RepresentationSettings;

  class RenderView;
  class View2D;
  class View3D;

  class Representation;
  using RepresentationPtr   = Representation *;
  using RepresentationList  = QList<RepresentationPtr>;
  using RepresentationSPtr  = std::shared_ptr<Representation>;
  using RepresentationSList = QList<RepresentationSPtr>;

  class EspinaGUI_EXPORT Representation
  : public QObject
  {
  public:
    using Type = QString;

    enum RenderableViews
    {
      RENDERABLEVIEW_SLICE     = 0x1,
      RENDERABLEVIEW_VOLUME    = 0x2,
      RENDERABLEVIEW_UNDEFINED = 0x4
    };
    Q_DECLARE_FLAGS(RenderableView, RenderableViews);

  public:
    /** \brief Representation class constructor.
     * \param[in] view, raw pointer of the RenderView the representation will be show.
     *
     */
    explicit Representation(RenderView *view);

    /** \brief Representation class virtual destructor.
     *
     */
    virtual ~Representation()
    {}

    /** \brief Sets the representation as active/inactive for the given view.
     * \param[in] value, true to set to active false otherwise.
     * \param[in] view, raw pointer of the view containing the representation or a clone instance.
     *
     */
    void setActive(bool value, RenderView *view = nullptr);

    /** \brief Returns whether or not the user has selected this representation to be displayed.
     *
     * Representation will only be currently displayed if it is visibility has also
     * been set to true.
     *
     */
    bool isActive() const
    { return m_active; }

    /** \brief Returns the type of the representation.
     *
     */
    Type type() const
    { return m_type; }

    /** \brief Creates and returns the raw pointer of the settings widget for this representation.
     *
     */
    virtual RepresentationSettings *settingsWidget() = 0;

    /** \brief Serializes and returns the settings for this widget.
     *
     */
    virtual QString serializeSettings();

    /** \brief Restores the settings of the widget with the given serialization.
     * \param[in] settings, text string.
     *
     */
    virtual void restoreSettings(QString settings);

    /** \brief Sets the color of the representation.
     * \param[in] color.
     *
     */
    virtual void setColor(const QColor &color)
    { m_color = color; }

    /** \brief Returns the color of the representation.
     *
     */
    virtual QColor color() const
    { return m_color; }

    /** \brief Sets the brightness of the representation.
     *
     * Brightness value in range [-1,1]
     *
     */
    virtual void setBrightness(double value)
    { m_brightness = value; }

    /** \brief Returns the brightness of the representation.
     *
     * Brightness value in range [-1,1]
     *
     */
    double brightness() const
    { return m_brightness; }

    /** \brief Sets the contrast of the representation.
     *
     * Contrast value in range [0,2]
     *
     */
    virtual void setContrast(double value)
    { m_contrast = value; }

    /** \brief Returns the contrast of the representation.
     *
     * Contrast value in range [0,2]
     *
     */
    double contrast() const
    { return m_contrast; }

    /** \brief Sets the opacity of the representation.
     *
     * Opacity value in range [0,1]
     *
     */
    virtual void setOpacity(double value)
    { m_opacity = value; }

    /** \brief Returns the opacity of the representation.
     *
     * Opacity value in range [0,1]
     *
     */
    double opacity() const
    { return m_opacity; }

    /** \brief Sets the highlight of the representation.
     * \param[in] highlight, true to set highlighted false otherwise.
     *
     */
    virtual void setHighlighted(bool highlighted)
    { m_highlight = highlighted; }

    /** \brief Returns true if the representation is highlighted.
     *
     */
    bool isHighlighted() const
    { return m_highlight; }

    /** \brief Sets representation visibility.
     * \param[in] visible, true to set to visible false otherwise.
     *
     * Inactive representations are never visible, even visibility is set to true
     *
     */
    void setVisible(bool visible);

    /** \brief Returns true if the representation is set to visible false otherwise.
     *
     * Inactive representations are never visible, even visibility is set to true
     *
     */
    bool isVisible() const
    { return m_visible && m_active; }

    /** \brief Returns true if the specified point is inside the segmentation the representation belongs to.
     * \param[in] point, point to check.
     *
     */
    virtual bool isInside(const NmVector3& point) const = 0;

    /** \brief Returns the type of view this representation can render on.
     *
     */
    virtual RenderableView canRenderOnView() const = 0;

    /** \brief Creates and returns a smart pointer of an instance of this representation for the given view.
     * \param[in] view, view 2d raw pointer.
     *
     */
    RepresentationSPtr clone(View2D *view);

    /** \brief Creates and returns a smart pointer of an instance of this representation for the given view.
     * \param[in] view, view 3d raw pointer.
     *
     */
    RepresentationSPtr clone(View3D *view);

    /** \brief Returns true if the specified actor belongs to this representation.
     * \param[in] actor, vtkProp raw pointer.
     *
     */
    virtual bool hasActor(vtkProp *actor) const = 0;

    /** \brief Updates the representation.
     *
     */
    virtual void updateRepresentation() = 0;

    /** \brief Returns the list of actors that comprise this representation.
     *
     */
    virtual QList<vtkProp*> getActors() = 0;

    /** \brief Returns true if the representation must change when the crosshair changes.
     *
     */
    virtual bool crosshairDependent() const = 0;

    /** \brief Returns true if the representation need an update of it's actors.
     *
     */
    virtual bool needUpdate() const = 0;

    /** \brief Sets the crosshair point position for this representation.
     * \param[in] point, crosshair point.
     *
     */
    void setCrosshairPoint(const NmVector3& point)
    { m_crosshair = point; }

    /** \brief Returns the crosshair point for this representation.
     *
     */
    NmVector3 crosshairPoint() const
    { return m_crosshair; }

  protected:
    /** \brief Helper method to return an smart pointer of an instance of this representation for the specified view.
     * \param[in] view, view 2d raw pointer.
     *
     */
    virtual RepresentationSPtr cloneImplementation(View2D *view) = 0;

    /** \brief Helper method to return an smart pointer of an instance of this representation for the specified view.
     * \param[in] view, view 3d raw pointer.
     *
     */
    virtual RepresentationSPtr cloneImplementation(View3D *view) = 0;

    /** \brief Helper method to update the visibility of the representation.
     * \param[in] visible, true to set the representation to visible false otherwise.
     *
     */
    virtual void updateVisibility(bool visible) = 0;

    /** \brief Sets the type of the representation.
     * \param[in] type, representation type.
     *
     */
    void setType(const Representation::Type &type)
    { m_type = type; }

  protected:
    double m_brightness;
    double m_contrast;
    double m_opacity;
    QColor m_color;
    bool   m_highlight;

    RenderView       *m_view;
    NmVector3         m_crosshair;
    mutable TimeStamp m_lastUpdatedTime;

    RepresentationSList m_clones;

  private:
    bool m_active;
    bool m_visible;
    Type m_type;
  };

  using RepresentationTypeList = QList<Representation::Type>;

  Q_DECLARE_OPERATORS_FOR_FLAGS(Representation::RenderableView)
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_H
