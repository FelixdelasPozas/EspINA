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


#include "AddSample.h"

#include <Core/Model/Sample.h>
#include <Core/Model/EspinaModel.h>

#include <QDebug>

using namespace EspINA;

//------------------------------------------------------------------------
AddSample::AddSample(SampleSPtr  sample,
                     EspinaModelSPtr  model,
                     QUndoCommand    *parent)
: QUndoCommand(parent)
, m_model(model)
, m_sample(sample)
{}

//------------------------------------------------------------------------
AddSample::~AddSample()
{
  qDebug() << "********************************************************";
  qDebug() << "            Destroying Add Sample Action";
  qDebug() << "********************************************************";
}


//------------------------------------------------------------------------
void AddSample::redo()
{
  m_model->addSample(m_sample);
}

//------------------------------------------------------------------------
void AddSample::undo()
{
  m_model->removeSample(m_sample);
}
