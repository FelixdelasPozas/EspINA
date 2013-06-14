/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef CONTOURRENDERER_H_
#define CONTOURRENDERER_H_

// EspINA
#include "MeshRenderer.h"
#include <Core/Model/Output.h>

namespace EspINA
{
  
  class ContourRenderer
  : public MeshRenderer
  {
    public:
      ContourRenderer(QObject* parent = 0);
      virtual ~ContourRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/contour.png"); }
      virtual const QString name()    const   { return "Contour"; }
      virtual const QString tooltip() const   { return "Segmentation's Contours"; }

      virtual void addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep);
      virtual void removeRepresentation(GraphicalRepresentationSPtr rep);
      virtual bool managesRepresentation(GraphicalRepresentationSPtr rep);

      virtual IRendererSPtr clone()                     { return IRendererSPtr(new ContourRenderer()); }
      virtual RendererType getRenderType()              { return RendererType(RENDERER_SLICEVIEW); }

      virtual ViewManager::Selection pick(int x,
                                          int y,
                                          Nm z,
                                          vtkSmartPointer<vtkRenderer> renderer,
                                          RenderabledItems itemType = RenderabledItems(),
                                          bool repeat = false);

      virtual void setView(EspinaRenderView* view);

      void RendererEventCallbackFunction(vtkObject* caller,
                                         long unsigned int eventId,
                                         void* callData);
    private:
      inline void computeWidth();

      int m_width;
  };

} /* namespace EspINA */
#endif /* CONTOURRENDERER_H_ */
