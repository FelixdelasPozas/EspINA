/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include "RepresentationRenderer.h"

//-----------------------------------------------------------------------------
bool EspINA::canRender(RepresentationRendererSPtr renderer, RenderableType type)
{
  return renderer->renderableItems().testFlag(type);
}

//-----------------------------------------------------------------------------
EspINA::RepresentationRendererSPtr EspINA::representationRenderer(RendererSPtr renderer)
{
  auto repRenderer = std::dynamic_pointer_cast<EspINA::RepresentationRenderer>(renderer);
  Q_ASSERT(repRenderer);
  return repRenderer;
}
