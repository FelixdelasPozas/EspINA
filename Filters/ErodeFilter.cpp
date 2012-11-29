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


#include "ErodeFilter.h"

#include <Core/Model/EspinaFactory.h>

#include <QDebug>

const QString ErodeFilter::TYPE = "EditorToolBar::ErodeFilter";

ErodeFilter::ErodeFilter(Filter::NamedInputs inputs,
                             ModelItem::Arguments args)
: MorphologicalEditionFilter(inputs, args)
{
//   qDebug() << TYPE << "arguments" << m_args;
}


//-----------------------------------------------------------------------------
ErodeFilter::~ErodeFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void ErodeFilter::run()
{
  Q_ASSERT(m_inputs.size() == 1);
  m_input = m_inputs.first()->toITK();

  qDebug() << "Compute Image Erode";
  StructuringElementType ball;
  ball.SetRadius(m_params.radius());
  ball.CreateStructuringElement();

  m_filter = FilterType::New();
  m_filter->SetInput(m_input);
  m_filter->SetKernel(ball);
  m_filter->SetObjectValue(SEG_VOXEL_VALUE);
  m_filter->Update();

  m_outputs.clear();
  SegmentationVolume::Pointer segVolume(new SegmentationVolume(m_filter->GetOutput()));
  m_outputs << Output(this, 0, segVolume);

  emit modified(this);
}

//-----------------------------------------------------------------------------
QVariant ErodeFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}