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
#include "ViewItem.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Input.h"

using namespace ESPINA;

//------------------------------------------------------------------------
ViewItem::ViewItem(InputSPtr input)
: m_input{input}
{
  connect(output().get(), SIGNAL(modified()),
          this,           SLOT(onOutputModified()));
}

//------------------------------------------------------------------------
ViewItem::~ViewItem()
{
}

//------------------------------------------------------------------------
OutputSPtr ViewItem::output()
{
  return m_input->output();
}

//------------------------------------------------------------------------
const OutputSPtr ViewItem::output() const
{
  return m_input->output();
}

// //------------------------------------------------------------------------
// DataSPtr ViewItem::data(Data::Type type)
// {
//   return m_input->output()->data(type);
// }
//
// //------------------------------------------------------------------------
// const DataSPtr ViewItem::data(Data::Type type) const
// {
//   return m_input->output()->data(type);
// }

//------------------------------------------------------------------------
void ViewItem::changeOutput(InputSPtr input)
{
  if (m_input->filter())
  {
    disconnect(output().get(), SIGNAL(modified()),
               this, SLOT(onOutputModified()));

    if (analysis())
    {
      analysis()->removeFilterContentRelation(m_input->filter(), this);
      analysis()->removeIfIsolated(m_input->filter());
    }
  }

  m_input = input;

  if (m_input->filter())
  {
    connect(output().get(), SIGNAL(modified()),
            this, SLOT(onOutputModified()));

    if (analysis())
    {
      analysis()->addIfNotExists(m_input->filter());
      analysis()->addFilterContentRelation(m_input->filter(), this);
    }
  }

  onOutputModified();
}

//------------------------------------------------------------------------
void ViewItem::changeOutput(FilterSPtr filter, Output::Id outputId)
{
  changeOutput(getInput(filter, outputId));
}
