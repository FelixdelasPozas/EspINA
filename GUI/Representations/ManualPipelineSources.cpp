/*

 Copyright (C) 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// ESPINA
#include "ManualPipelineSources.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
ManualPipelineSources::ManualPipelineSources(GUI::View::ViewState &viewState)
: PipelineSources(viewState)
{
}

//-----------------------------------------------------------------------------
void ManualPipelineSources::addSource(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame)
{
  insert(sources);
}

//-----------------------------------------------------------------------------
void ManualPipelineSources::removeSource(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame)
{
  remove(sources);
}

//-----------------------------------------------------------------------------
void ManualPipelineSources::updateRepresentation(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame)
{
  onRepresentationsInvalidated(sources, frame);
}
