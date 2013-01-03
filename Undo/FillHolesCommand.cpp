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

#include <Filters/FillHolesFilter.h>
#include <Core/Model/EspinaModel.h>

#include <QApplication>

using namespace EspINA;

//-----------------------------------------------------------------------------
FillHolesCommand::FillHolesCommand(SegmentationList inputs, EspinaModel *model)
: m_model(model)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  foreach(SegmentationPtr seg, inputs)
  {
    Filter::NamedInputs inputs;
    Filter::Arguments args;
    inputs[FillHolesFilter::INPUTLINK] = seg->filter();
    args[Filter::INPUTS] = Filter::NamedInput(FillHolesFilter::INPUTLINK, seg->outputId());
    FilterSPtr filter(new FillHolesFilter(inputs, args));
    filter->update();
    m_segmentations  << m_model->findSegmentation(seg);
    m_newConnections << Connection(filter, 0);
    m_oldConnections << Connection(seg->filter(), seg->outputId());
  }
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
FillHolesCommand::~FillHolesCommand()
{

}

//-----------------------------------------------------------------------------
void FillHolesCommand::redo()
{
  for(int i=0; i<m_newConnections.size(); i++)
  {
    SegmentationSPtr seg = m_segmentations[i];
    Connection oldConnection  = m_oldConnections[i];
    Connection newConnection  = m_newConnections[i];

    m_model->removeRelation(oldConnection.first, seg, Filter::CREATELINK);
    m_model->addFilter(newConnection.first);
    m_model->addRelation(oldConnection.first, newConnection.first, FillHolesFilter::INPUTLINK);
    m_model->addRelation(newConnection.first, seg, Filter::CREATELINK);
    seg->changeFilter(newConnection.first, newConnection.second);
    seg->notifyModification(true);
  }
}

//-----------------------------------------------------------------------------
void FillHolesCommand::undo()
{
  for(int i=0; i<m_newConnections.size(); i++)
  {
    SegmentationSPtr seg = m_segmentations[i];
    Connection oldConnection  = m_oldConnections[i];
    Connection newConnection  = m_newConnections[i];

    m_model->removeRelation(newConnection.first, seg, Filter::CREATELINK);
    m_model->removeRelation(oldConnection.first, newConnection.first, FillHolesFilter::INPUTLINK);
    m_model->removeFilter(newConnection.first);
    m_model->addRelation(oldConnection.first, seg, Filter::CREATELINK);
    seg->changeFilter(oldConnection.first, oldConnection.second);
    seg->notifyModification(true);
  }
}