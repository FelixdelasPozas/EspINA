/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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

#ifndef ESPINA_REPRESENTATION_H
#define ESPINA_REPRESENTATION_H

#include "EspinaGUI_Export.h"

// Qt
#include <QList>
#include <QString>
#include <QColor>

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/NmVector3.h>

class vtkProp3D;
class vtkProp;
class vtkMatrix4x4;
class vtkLookupTable;

namespace EspINA
{
  class RepresentationSettings;

  class RenderView;
  class SliceView;
  class VolumeView;

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
    explicit Representation(RenderView *view);

    virtual ~Representation(){}

    void setActive(bool value, RenderView *view = NULL);

    /// Whether or not the user has selected this representation to be displayed
    /// Representation will only be currently displayed if it is visibility has also
    /// been set to true
    bool isActive() const
    { return m_active; }

    void setLabel(const QString &label)
    { m_label = label; }

    QString label() const
    { return m_label; }

    virtual RepresentationSettings *settingsWidget() = 0;

    virtual QString serializeSettings();

    virtual void restoreSettings(QString settings);

    virtual void setColor(const QColor &color)
    { m_color = color; }

    virtual QColor color() const
    { return m_color; }

    /// Brightness value in range [-1,1]
    virtual void setBrightness(double value)
    { m_brightness = value; }

    /// Brightness value in range [-1,1]
    double brightness() const
    { return m_brightness; }

    /// Contrast value in range [0,2]
    virtual void setContrast(double value)
    { m_contrast = value; }

    /// Contrast value in range [0,2]
    double contrast() const
    { return m_contrast; }

    /// Opacity value in range [0,1]
    virtual void setOpacity(double value)
    { m_opacity = value; }

    /// Opacity value in range [0,1]
    double opacity() const
    { return m_opacity; }

    virtual void setHighlighted(bool highlighted)
    { m_highlight = highlighted; }

    bool isHighlighted() const
    { return m_highlight; }

    /// Set representation visibility
    /// Innactive representations are never visible, even visibility is set to true
    void setVisible(bool visible);

    /// Whether or not the representation is displayed
    /// Innactive representations are never visible, even visibility is set to true
    bool isVisible() const
    { return m_visible && m_active; }

    virtual bool isInside(const NmVector3& point) const = 0;

    virtual RenderableView canRenderOnView() const { return RENDERABLEVIEW_UNDEFINED; };

    RepresentationSPtr clone(SliceView *view);
    RepresentationSPtr clone(VolumeView *view);

    virtual bool hasActor(vtkProp *actor) const = 0;

    virtual void updateRepresentation() = 0;

    virtual QList<vtkProp*> getActors() = 0;

    void setCrosshairPoint(const NmVector3& point)
    { m_crosshair = point; onCrosshairChanged(m_crosshair); }

    NmVector3 crosshairPoint() const
    { return m_crosshair; }

  protected:
    virtual RepresentationSPtr cloneImplementation(SliceView *view) = 0;
    virtual RepresentationSPtr cloneImplementation(VolumeView *view) = 0;

    virtual void updateVisibility(bool visible) = 0;

    virtual void onCrosshairChanged(const NmVector3& point) = 0;


  protected:
    double m_brightness;
    double m_contrast;
    double m_opacity;
    QColor m_color;
    bool   m_highlight;

    RenderView*   m_view;
    NmVector3     m_crosshair;

    RepresentationSList m_clones;

  private:
    bool    m_active;
    bool    m_visible;
    QString m_label;
  };

  using RepresentationTypeList = QList<Representation::Type>;

  Q_DECLARE_OPERATORS_FOR_FLAGS(Representation::RenderableView)
} // namespace EspINA

#endif // ESPINA_REPRESENTATION_H
