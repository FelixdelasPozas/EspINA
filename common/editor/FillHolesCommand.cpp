/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "FillHolesCommand.h"

#include "FillHolesFilter.h"
#include <EspinaCore.h>

//-----------------------------------------------------------------------------
FillHolesCommand::FillHolesCommand(SegmentationList inputs)
: m_segmentations(inputs)
{
  foreach(Segmentation *seg, m_segmentations)
  {
    Filter::NamedInputs inputs;
    Filter::Arguments args;
    inputs[FillHolesFilter::INPUTLINK] = seg->filter();
    args[Filter::INPUTS] = FillHolesFilter::INPUTLINK + "_" + QString::number(seg->outputNumber());
    Filter *filter = new FillHolesFilter(inputs, args);
    filter->update();
    m_newConnections << Connection(filter, 0);
    m_oldConnections << Connection(seg->filter(), seg->outputNumber());
  }
}

//-----------------------------------------------------------------------------
FillHolesCommand::~FillHolesCommand()
{

}

//-----------------------------------------------------------------------------
void FillHolesCommand::redo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  for(unsigned int i=0; i<m_newConnections.size(); i++)
  {
    Segmentation *seg        = m_segmentations[i];
    Connection oldConnection = m_oldConnections[i];
    Connection newConnection = m_newConnections[i];

    model->removeRelation(oldConnection.first, seg, CREATELINK);
    model->addFilter(newConnection.first);
    model->addRelation(oldConnection.first, newConnection.first, FillHolesFilter::INPUTLINK);
    model->addRelation(newConnection.first, seg, CREATELINK);
    seg->changeFilter(newConnection.first, newConnection.second);
    seg->notifyModification(true);
  }
}

//-----------------------------------------------------------------------------
void FillHolesCommand::undo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  for(unsigned int i=0; i<m_newConnections.size(); i++)
  {
    Segmentation *seg        = m_segmentations[i];
    Connection oldConnection = m_oldConnections[i];
    Connection newConnection = m_newConnections[i];

    model->removeRelation(newConnection.first, seg, CREATELINK);
    model->removeRelation(oldConnection.first, newConnection.first, FillHolesFilter::INPUTLINK);
    model->removeFilter(newConnection.first);
    model->addRelation(oldConnection.first, seg, CREATELINK);
    seg->changeFilter(oldConnection.first, oldConnection.second);
    seg->notifyModification(true);
  }
}