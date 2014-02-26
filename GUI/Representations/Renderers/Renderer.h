/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Pe�a Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_RENDERER_H
#define ESPINA_RENDERER_H

#include "EspinaGUI_Export.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/Representations/Representation.h>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/View/SelectableView.h>

// Qt
#include <QString>
#include <QIcon>

// VTK
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

namespace EspINA
{
  class RenderView;

  class Renderer;
  using RendererPtr   = Renderer *;
  using RendererSPtr  = std::shared_ptr<Renderer>;
  using RendererList  = QList<RendererPtr>;
  using RendererSList = QList<RendererSPtr>;

  enum RenderableType {
    CHANNEL      = 0x1,
    SEGMENTATION = 0x2,
  };

  Q_DECLARE_FLAGS(RenderableItems, RenderableType);

  enum RendererType
  {
    RENDERER_UNDEFINED_VIEW = 0x1,
    RENDERER_VIEW2D         = 0x2,
    RENDERER_VIEW3D         = 0x4
  };

  Q_DECLARE_FLAGS(RendererTypes, RendererType);

  /// Base class which define the API to render and manage
  /// item visibily in Espina Views (currently only supported
  /// for View3D class)
  class EspinaGUI_EXPORT Renderer
  : public QObject
  {
    Q_OBJECT
  public:
    virtual ~Renderer(){}

    /// Following methods are used by view settings' panel and the
    /// view itself to create the corresponding UI to manage rendering
    virtual const QString name()    const { return QString(); }
    virtual const QString tooltip() const { return QString(); }
    virtual const QIcon icon()      const { return QIcon(); }

    /// sets renderer
    virtual void setView(RenderView* view) { m_view = view; }

    virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep) = 0;
    virtual void removeRepresentation(RepresentationSPtr rep) = 0;
    virtual bool hasRepresentation(RepresentationSPtr rep) = 0;
    virtual bool managesRepresentation(RepresentationSPtr rep) = 0;

    virtual RendererSPtr clone() = 0;

    // get number of vtkActors added to vtkRendered from this Renderer
    virtual unsigned int numberOfvtkActors() = 0;

    virtual bool isHidden() { return !m_enable; }

    virtual RenderableItems renderableItems() { return RenderableItems(); }

    virtual RendererTypes renderType() { return RendererTypes(RENDERER_UNDEFINED_VIEW); }

    // naive item filtering, to be modified/enhanced in the future
    virtual bool canRender(ItemAdapterPtr item)
    { return true; }

    // return the number of elements actually been managed by this renderer
    virtual int numberOfRenderedItems() = 0;

    virtual ViewItemAdapterList pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType = RenderableItems(), bool repeat = false) = 0;

    virtual NmVector3 pickCoordinates() const = 0;

  protected:
    // Hide/Show all items rendered by the Renderer
    virtual void hide() = 0;
    virtual void show() = 0;

  public slots:
    virtual void setEnable(bool value)
    {
      if (m_view)
      {
        if (value)
        {
          show();
        }
        else
        {
          hide();
        }

        m_enable = value;

        emit enabled(m_enable);
      }
    }

  signals:
    void renderRequested();
    void enabled(bool);

  protected:
    explicit Renderer(QObject* parent = 0)
    : m_enable(false)
    , m_view (nullptr)
    {}

  protected:
    bool m_enable;
    QMap<ViewItemAdapterPtr, RepresentationSList> m_representations;
    RenderView* m_view;
  };

  class EspinaGUI_EXPORT ChannelRenderer
  : public Renderer
  {
    public:
      explicit ChannelRenderer(QObject* parent = 0): Renderer(parent) {}
      virtual ~ChannelRenderer() {}

      virtual void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]) = 0;
      virtual void setCrosshair(NmVector3 point) = 0;
      virtual void setPlanePosition(Plane plane, Nm dist) = 0;
  };


  bool EspinaGUI_EXPORT canRender(RendererSPtr renderer, RendererType type);
  bool EspinaGUI_EXPORT canRender(RendererSPtr renderer, RenderableType type);

  Q_DECLARE_OPERATORS_FOR_FLAGS(RenderableItems)
  Q_DECLARE_OPERATORS_FOR_FLAGS(RendererTypes)
}// namespace EspINA

#endif // ESPINA_RENDERER_H
