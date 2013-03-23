/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#ifndef APPOSITIONSURFACERENDERER_H
#define APPOSITIONSURFACERENDERER_H

// EspINA
#include <GUI/Renderers/MeshRenderer.h>
#include <Core/Extensions/AppositionSurfaceExtension.h>
#include <QDebug>

namespace EspINA
{
  class ViewManager;

  class AppositionSurfaceRenderer
  : public MeshRenderer
  {

  public:
    explicit AppositionSurfaceRenderer(ViewManager *vm, QObject *parent = 0): MeshRenderer(vm, parent) {};
    virtual ~AppositionSurfaceRenderer() {};

    virtual const QIcon   icon()    const { return QIcon(":/AppSurface.svg"); }
    virtual const QString name()    const { return tr("Sinaptic Apposition Surface"); }
    virtual const QString tooltip() const { return tr("Sinaptic Apposition Surfaces Renderer"); }

    virtual IRendererSPtr clone() { return IRendererSPtr(new AppositionSurfaceRenderer(m_viewManager)); }

  protected:
    virtual bool itemCanBeRendered(ModelItemPtr item)
    {
      if (item->type() != EspINA::SEGMENTATION)
        return false;

      QString fullTaxonomy = SegmentationPtr(item)->taxonomy()->qualifiedName();
      return (fullTaxonomy.startsWith(SAS+"/") || (fullTaxonomy.compare(SAS) == 0));
    }
  };

  typedef QSharedPointer<AppositionSurfaceRenderer> AppositionSurfaceRendererSPtr;
}

#endif // APPOSITIONSURFACERENDERER_H
