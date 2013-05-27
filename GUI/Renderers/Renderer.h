/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef IRENDERER
#define IRENDERER

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/ViewManager.h>
#include <GUI/Representations/GraphicalRepresentation.h>

// Qt
#include <QString>
#include <QIcon>

// VTK
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

namespace EspINA
{
  class EspinaRenderView;

  class IRenderer;
  typedef boost::shared_ptr<IRenderer> IRendererSPtr;
  /// Base class which define the API to render and manage
  /// item visibily in Espina Views (currently only supported
  /// for VolumeView class)

  class IRenderer
  : public QObject
  {
    Q_OBJECT
  public:
    virtual ~IRenderer(){}

    /// Following methods are used by view settings' panel and the
    /// view itself to create the corresponding UI to manage rendering
    virtual const QString name() const       { return QString(); }
    virtual const QString tooltip() const    { return QString(); }
    virtual const QIcon icon() const         { return QIcon(); }

    /// sets renderer
    virtual void setView(EspinaRenderView* view) { m_view = view; }

    virtual void addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep) = 0;
    virtual void removeRepresentation(GraphicalRepresentationSPtr rep) = 0;
    virtual bool hasRepresentation(GraphicalRepresentationSPtr rep) = 0;
    virtual bool managesRepresentation(GraphicalRepresentationSPtr rep) = 0;

    // Hide/Show all items rendered by the Renderer
    virtual void hide() = 0;
    virtual void show() = 0;

    virtual IRendererSPtr clone() = 0;

    // get number of vtkActors added to vtkRendered from this Renderer
    virtual unsigned int getNumberOfvtkActors() = 0;

    virtual bool isHidden() { return !m_enable; }

    Q_DECLARE_FLAGS(RenderabledItems, EspINA::ModelItemType);
    virtual RenderabledItems getRenderableItemsType() { return RenderabledItems(); }

    enum RendererTypes
    {
      RENDERER_UNDEFINED_VIEW = 0x1,
      RENDERER_SLICEVIEW      = 0x2,
      RENDERER_VOLUMEVIEW     = 0x4
    };
    Q_DECLARE_FLAGS(RendererType, RendererTypes);

    virtual RendererType getRenderType() { return RendererType(RENDERER_UNDEFINED_VIEW); }

    // naive item filtering, to be modified/enhanced in the future
    virtual bool itemCanBeRendered(ModelItemPtr item) { return true; }

    // return the number of elements actually been managed by this renderer
    virtual int itemsBeenRendered() = 0;

    virtual ViewManager::Selection pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderabledItems itemType = RenderabledItems(), bool repeat = false) = 0;
    virtual void getPickCoordinates(Nm *point) = 0;

  public slots:
    virtual void setEnable(bool value)
    {
      if (value)
        show();
      else
        hide();

      m_enable = value;
    }

  signals:
    void renderRequested();

  protected:
    explicit IRenderer(QObject* parent = 0)
    : m_enable(false)
    , m_view (NULL)
    {}

  protected:
    bool m_enable;
    QMap<PickableItemPtr, GraphicalRepresentationSList> m_representations;
    EspinaRenderView *m_view;
  };

  typedef QList<IRenderer *>   IRendererList;
  typedef QList<IRendererSPtr> IRendererSList;

  Q_DECLARE_OPERATORS_FOR_FLAGS(IRenderer::RenderabledItems)
  Q_DECLARE_OPERATORS_FOR_FLAGS(IRenderer::RendererType)
}// namespace EspINA



#endif // IRENDERER
