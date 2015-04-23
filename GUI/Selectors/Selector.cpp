/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "GUI/Selectors/Selector.h"

#include "GUI/Model/SampleAdapter.h"
#include "GUI/Model/ChannelAdapter.h"
#include "GUI/Model/SegmentationAdapter.h"
#include <GUI/Model/Utils/SegmentationUtils.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

//-----------------------------------------------------------------------------
void Selector::setSelectionTag(const Selector::SelectionTag tag, bool selectable)
{
  if (selectable)
  {
    m_flags |= tag;
  }
  else
  {
    m_flags &= !tag;
  }
}

//-----------------------------------------------------------------------------
bool Selector::IsValid(NeuroItemAdapterPtr item, Selector::SelectionFlags flags)
{
  if(isChannel(item))
  {
    return flags.testFlag(CHANNEL) || flags.testFlag(SAMPLE);
  }

  if(isSegmentation(item))
  {
    return flags.testFlag(SEGMENTATION);
  }

  if(isSample(item))
  {
    return flags.testFlag(SAMPLE);
  }

  return false;
}

