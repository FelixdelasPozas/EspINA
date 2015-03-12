/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "ViewItemAdapter.h"
#include "ModelAdapter.h"
#include <Core/Analysis/NeuroItem.h>

using namespace ESPINA;

//------------------------------------------------------------------------
ViewItemAdapter::ViewItemAdapter(ViewItemSPtr item)
: NeuroItemAdapter  {item}
, m_viewItem        {item}
, m_isSelected      {false}
, m_isVisible       {true}
{
  connect(output().get(), SIGNAL(modified()),
          this,           SLOT(onOutputModified()));
}

//------------------------------------------------------------------------
void ViewItemAdapter::setTemporalRepresentation(RepresentationPipelineSPtr pipeline)
{
  m_temporalRepresentation = pipeline;
}

//------------------------------------------------------------------------
void ViewItemAdapter::clearTemporalRepresentation()
{
  m_temporalRepresentation.reset();
}

//------------------------------------------------------------------------
RepresentationPipelineSPtr ViewItemAdapter::temporalRepresentation() const
{
  return m_temporalRepresentation;
}

//------------------------------------------------------------------------
void ViewItemAdapter::invalidateRepresentations()
{
  emit representationsInvalidated(this);
}

//------------------------------------------------------------------------
void ViewItemAdapter::changeOutput(InputSPtr input)
{
  disconnect(output().get(), SIGNAL(modified()),
             this,           SLOT(onOutputModified()));

  changeOutputImplementation(input);

  connect(output().get(), SIGNAL(modified()),
          this,           SLOT(onOutputModified()));

  // TODO: shall we deactivate the temporal representation if any?
  onOutputModified();
}

//------------------------------------------------------------------------
void ViewItemAdapter::onOutputModified()
{
  emit outputChanged(this);
}

//------------------------------------------------------------------------
ViewItemAdapterPtr ESPINA::viewItemAdapter(ItemAdapterPtr item)
{
  return static_cast<ViewItemAdapterPtr>(item);
}
