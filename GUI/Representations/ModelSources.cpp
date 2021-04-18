/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "ModelSources.h"
#include <Core/Utils/ListUtils.hxx>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
ModelSources::ModelSources(ModelAdapterSPtr model, GUI::View::ViewState &viewState)
: PipelineSources(viewState)
, m_model{model}
{
  connect(m_model.get(), SIGNAL(viewItemsAdded(ViewItemAdapterSList)),
          this,          SLOT(onSourcesAdded(ViewItemAdapterSList)));

  connect(m_model.get(), SIGNAL(viewItemsAboutToBeRemoved(ViewItemAdapterSList)),
          this,          SLOT(onSourcesRemoved(ViewItemAdapterSList)));

  connect(m_model.get(), SIGNAL(aboutToBeReset()),
          this,          SLOT(onReset()));
}

//-----------------------------------------------------------------------------
ModelSources::~ModelSources()
{
}

//-----------------------------------------------------------------------------
void ModelSources::onSourcesAdded(ViewItemAdapterSList sources)
{
  insert(rawList(sources));
}

//-----------------------------------------------------------------------------
void ModelSources::onSourcesRemoved(ViewItemAdapterSList sources)
{
  remove(rawList(sources));
}

//-----------------------------------------------------------------------------
void ModelSources::onReset()
{
  if (!isEmpty())
  {
    remove(sources(ItemAdapter::Type::CHANNEL)+sources(ItemAdapter::Type::SEGMENTATION));
  }
}