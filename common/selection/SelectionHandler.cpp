/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "SelectionHandler.h"

const IPicker::Tag IPicker::SAMPLE = "EspINA_Sample";
const IPicker::Tag IPicker::CHANNEL = "EspINA_Channel";
const IPicker::Tag IPicker::SEGMENTATION = "EspINA_Segmentation";

//-----------------------------------------------------------------------------
bool IPicker::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (m_succesor)
    return m_succesor->filterEvent(e, view);
  else
    return false;
}

//-----------------------------------------------------------------------------
void IPicker::setSelection(IPicker::PickList pickList)
{
//   qDebug("Selection Changed");
  emit itemsPicked(pickList);
}

//-----------------------------------------------------------------------------
void IPicker::abortPick()
{
//   qDebug("Selection Aborted");
  emit selectionAborted();
}

//-----------------------------------------------------------------------------
void IPicker::setPickable(QString type, bool pick)
{
  if (pick)
    m_filters.append(type);
  else
    m_filters.removeAll(type);
}
