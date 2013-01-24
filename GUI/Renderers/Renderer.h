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

#include <Core/EspinaTypes.h>

#include <QString>
#include <QIcon>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

namespace EspINA
{

  typedef QSharedPointer<IRenderer> IRendererSPtr;
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
    virtual void setVtkRenderer(vtkSmartPointer<vtkRenderer> renderer) {m_renderer = renderer;}

    // Return whether the item was rendered or not
    virtual bool addItem   (ModelItemPtr item) = 0;
    virtual bool updateItem(ModelItemPtr item) = 0;
    virtual bool removeItem(ModelItemPtr item) = 0;

    // Hide/Show all items rendered by the Renderer
    virtual void hide() = 0;
    virtual void show() = 0;

    // Remove all items rendered by the Renderer
    virtual void clean() = 0;

    virtual IRendererSPtr clone() = 0;

    // get number of vtkActors added to vtkRendered from this Renderer
    virtual unsigned int getNumberOfvtkActors() = 0;

    virtual bool isHidden() { return !m_enable; }

    // true if this renderer renders segmentations only
    virtual bool isASegmentationRenderer() { return false; };

    // naive item filtering, to be modified/enhanced in the future
    virtual bool itemCanBeRendered(ModelItemPtr item) { return true; }

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
    : m_renderer(NULL)
    , m_enable(false) {}

  protected:
    vtkSmartPointer<vtkRenderer> m_renderer;
    bool m_enable;

  };

  typedef QList<IRenderer *>   IRendererList;
  typedef QList<IRendererSPtr> IRendererSList;

}// namespace EspINA

#endif // IRENDERER
