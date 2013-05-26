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

#ifndef IESPINAREPRESENTATION_H
#define IESPINAREPRESENTATION_H

// Qt
#include <QList>
#include <QString>
#include <QColor>

// boost
#include <boost/shared_ptr.hpp>

// EspINA
#include <Core/Model/Output.h>
#include <Core/EspinaTypes.h>

class vtkProp3D;
class vtkProp;
class vtkMatrix4x4;
class vtkLookupTable;

namespace EspINA
{

  class EspinaRenderView;
  class SliceView;
  class VolumeView;

  class GraphicalRepresentation
  : public QObject
  {
  public:
    enum RenderableViews
    {
      RENDERABLEVIEW_SLICE,
      RENDERABLEVIEW_VOLUME,
      RENDERABLEVIEW_UNDEFINED
    };
    Q_DECLARE_FLAGS(RenderableView, RenderableViews);

  public:
    explicit GraphicalRepresentation(EspinaRenderView *view);

    virtual ~GraphicalRepresentation(){}

    void setActive(bool value, EspinaRenderView *view = NULL);

    /// Whether or not the user has selected this representation to be displayed
    /// Representation will only be currently displayed if it is visibility has also
    /// been set to true
    bool isActive() const
    { return m_active; }

    void setLabel(const QString &label)
    { m_label = label; }

    QString label() const
    { return m_label; }

    virtual void setColor(const QColor &color)
    { m_color = color; }

    QColor color() const
    { return m_color; }

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

    virtual bool isInside(Nm point[3]) = 0;

    virtual RenderableView canRenderOnView() const { return RENDERABLEVIEW_UNDEFINED; };

    GraphicalRepresentationSPtr clone(SliceView *view);
    GraphicalRepresentationSPtr clone(VolumeView *view);

    virtual bool hasActor(vtkProp *actor) const = 0;

    virtual void updateRepresentation() = 0;

    virtual QList<vtkProp*> getActors() = 0;

  protected:
    virtual GraphicalRepresentationSPtr cloneImplementation(SliceView *view) = 0;
    virtual GraphicalRepresentationSPtr cloneImplementation(VolumeView *view) = 0;

    virtual void updateVisibility(bool visible) = 0;

    vtkMatrix4x4 *slicingMatrix(SliceView *view) const;

  protected:
    QColor            m_color;
    bool              m_highlight;
    EspinaRenderView *m_view;
    GraphicalRepresentationSList m_clones;

  private:
    bool    m_active;
    bool    m_visible;
    QString m_label;
  };

  class ChannelGraphicalRepresentation
  : public GraphicalRepresentation
  {
  public:
    explicit ChannelGraphicalRepresentation(EspinaRenderView *view);

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

  protected:
    double            m_brightness;
    double            m_contrast;
    double            m_opacity;
  };

  typedef boost::shared_ptr<ChannelGraphicalRepresentation> ChannelGraphicalRepresentationSPtr;
  typedef QList<ChannelGraphicalRepresentationSPtr> ChannelGraphicalRepresentationList;

  class SegmentationGraphicalRepresentation
  : public GraphicalRepresentation
  {
  public:
    explicit SegmentationGraphicalRepresentation(EspinaRenderView *view)
    : GraphicalRepresentation(view) {}
  };

  typedef boost::shared_ptr<SegmentationGraphicalRepresentation> SegmentationGraphicalRepresentationSPtr;
  typedef QList<SegmentationGraphicalRepresentationSPtr> SegmentationGraphicalRepresentationList;

  Q_DECLARE_OPERATORS_FOR_FLAGS(GraphicalRepresentation::RenderableView)
} // namespace EspINA

#endif // IESPINAREPRESENTATION_H
