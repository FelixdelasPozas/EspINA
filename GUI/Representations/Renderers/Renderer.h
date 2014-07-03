/*
    
    Copyright (C) 2014  Jorge Pe�a Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_RENDERER_H
#define ESPINA_RENDERER_H

#include "GUI/EspinaGUI_Export.h"

// EspINA
#include <Core/EspinaTypes.h>
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

  /* \brief Flags that define the views supported by this renderer.
   *
   */
  enum RendererType
  {
    RENDERER_UNDEFINED_VIEW = 0x1,
    RENDERER_VIEW2D         = 0x2,
    RENDERER_VIEW3D         = 0x4
  };
  Q_DECLARE_FLAGS(RendererTypes, RendererType);

  class EspinaGUI_EXPORT Renderer
  : public QObject
  {
    Q_OBJECT
  public:
    /* \brief enum type class
     *
     */
    enum class Type: std::int8_t { Base = 1, Representation = 2, Other = 3 };

    virtual ~Renderer(){}

    /* \brief Returns the name of the renderer.
     *
     */
    virtual const QString name() const
    { return QString(); }

    /* \brief Returns the tooltip associated to GUI elements representing this renderer.
     *
     */
    virtual const QString tooltip() const
    { return QString(); }

    /* \brief Returns the icon associated to GUI elements representing this renderer.
     *
     */
    virtual const QIcon icon() const
    { return QIcon(); }

    /* \brief Initializes renderer.
     *
     */
    virtual void setView(RenderView* view)
    { m_view = view; }

    /* \brief Clones the class from a prototype.
     *
     */
    virtual RendererSPtr clone() const = 0;

    /* \brief Returns the number of vtkActors added to the view's vtkRenderer from this Renderer
     *
     */
    virtual unsigned int numberOfvtkActors() const = 0;

    /* \brief Returns true if the renderer is not enabled.
     *
     */
    virtual bool isHidden() const
    { return !m_enable; }

    /* \brief Returns flags describing the view supported by this renderer, that is,
     * views that this renderer can work with.
     */
    virtual RendererTypes renderType() const { return RendererTypes(RENDERER_UNDEFINED_VIEW); }

    /* \brief Return the number of elements actually been managed by this renderer (abstract items).
     *
     */
    virtual int numberOfRenderedItems() const = 0;

    /* \brief Return the type of render class.
     *
     */
    virtual Type type() const
    { return Type::Base; }

  protected:
    /* \brief Convenience methods to hide or show all elements of a renderer. To be used only by setEnable().
     *
     */
    virtual void hide() = 0;
    virtual void show() = 0;

  public slots:
    /* \brief Enables or disables the render, efectively showing or hiding all managed elements.
     *
     */
    virtual void setEnable(bool value)
    {
      if (m_view)
      {
        if (value)
          show();
        else
          hide();

        m_enable = value;
      }
    }

  signals:
    /* \brief Signal emitted when enabling/diabling the renderer to force an update in it's view.
     *
     */
    void renderRequested();

  protected:
    explicit Renderer(QObject* parent = 0)
    : m_enable(false)
    , m_view (nullptr)
    {}

  protected:
    bool        m_enable;
    RenderView* m_view;
  };

  /* \brief Returns true if the render can render in the view specified by RendererType.
   *
   */
  bool canRender(RendererSPtr renderer, RendererType type);

  /* \brief Returns true if the render can render in the views specified by RendererTypes.
   *
   */
  bool canRender(RendererSPtr renderer, RendererTypes types);

  Q_DECLARE_OPERATORS_FOR_FLAGS(RendererTypes)
}// namespace EspINA

#endif // ESPINA_RENDERER_H
