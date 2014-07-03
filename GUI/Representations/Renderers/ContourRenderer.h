/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CONTOUR_RENDERER_H_
#define ESPINA_CONTOUR_RENDERER_H_

#include "GUI/EspinaGUI_Export.h"

// EspINA
#include "MeshRenderer.h"

namespace EspINA
{
  class EspinaGUI_EXPORT ContourRenderer
  : public MeshRenderer
  {
    public:
      ContourRenderer(QObject* parent = 0);
      virtual ~ContourRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/contour.png"); }
      virtual const QString name()    const   { return "Contour"; }
      virtual const QString tooltip() const   { return "Segmentation's Contours"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool managesRepresentation(const QString &repName) const;

      virtual RendererSPtr clone() const           { return RendererSPtr(new ContourRenderer()); }
      virtual RendererTypes renderType() const     { return RendererTypes(RENDERER_VIEW2D); }

      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false);
  };

} // namespace EspINA
#endif // ESPINA_CONTOUR_RENDERER_H_
