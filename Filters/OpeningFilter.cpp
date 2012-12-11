/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#include "OpeningFilter.h"

#include <Core/Model/EspinaFactory.h>

#include <QDebug>

const QString OpeningFilter::TYPE = "EditorToolBar::OpeningFilter";

OpeningFilter::OpeningFilter(Filter::NamedInputs inputs,
                             ModelItem::Arguments args)
: MorphologicalEditionFilter(inputs, args)
{
//   qDebug() << TYPE << "arguments" << m_args;
}


//-----------------------------------------------------------------------------
OpeningFilter::~OpeningFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void OpeningFilter::run()
{
  Q_ASSERT(m_inputs.size() == 1);
  m_input = m_inputs.first()->toITK();

  qDebug() << "Compute Image Opening";
  StructuringElementType ball;
  ball.SetRadius(m_params.radius());
  ball.CreateStructuringElement();

  m_filter = FilterType::New();
  m_filter->SetInput(m_input);
  m_filter->SetKernel(ball);
  m_filter->SetForegroundValue(SEG_VOXEL_VALUE);
  m_filter->Update();

  createOutput(0, m_filter->GetOutput());

  emit modified(this);
}

//-----------------------------------------------------------------------------
QVariant OpeningFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}