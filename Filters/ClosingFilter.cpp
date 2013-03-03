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


#include "ClosingFilter.h"

#include <Core/Model/EspinaFactory.h>
#include <itkImageRegionConstIterator.h>
#include <QDebug>

using namespace EspINA;

//-----------------------------------------------------------------------------
ClosingFilter::ClosingFilter(NamedInputs inputs,
                             Arguments   args,
                             FilterType  type)
: MorphologicalEditionFilter(inputs, args, type)
, m_filter(NULL)
{
}

//-----------------------------------------------------------------------------
ClosingFilter::~ClosingFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void ClosingFilter::run()
{
  Q_ASSERT(m_inputs.size() == 1);
  m_input = m_inputs.first()->toITK();

  StructuringElementType ball;
  ball.SetRadius(m_params.radius());
  ball.CreateStructuringElement();

  m_filter = BinaryClosingFilter::New();
  m_filter->SetInput(m_input);
  m_filter->SetKernel(ball);
  m_filter->SetForegroundValue(SEG_VOXEL_VALUE);
  m_filter->Update();

  m_isOutputEmpty = true;
  typedef itk::ImageRegionConstIterator<itkVolumeType> ImageIterator;
  ImageIterator it(m_filter->GetOutput(), m_filter->GetOutput()->GetLargestPossibleRegion());
  it.GoToBegin();
  for(it.GoToBegin(); !it.IsAtEnd(); ++it)
    if (it.Get() == SEG_VOXEL_VALUE)
    {
      m_isOutputEmpty = false;
      break;
    }

  if (!m_isOutputEmpty)
    createOutput(0, m_filter->GetOutput());

  emit modified(this);
}
