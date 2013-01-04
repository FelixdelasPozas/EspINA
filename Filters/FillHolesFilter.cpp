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


#include "FillHolesFilter.h"

using namespace EspINA;

const QString FillHolesFilter::INPUTLINK = "Input";

//-----------------------------------------------------------------------------
FillHolesFilter::FillHolesFilter(NamedInputs inputs,
                                 Arguments   args,
                                 FilterType  type)
: SegmentationFilter(inputs, args, type)
{
}

//-----------------------------------------------------------------------------
FillHolesFilter::~FillHolesFilter()
{
}

//-----------------------------------------------------------------------------
bool FillHolesFilter::needUpdate() const
{
  bool update = Filter::needUpdate();

  if (!update)
  {
    Q_ASSERT(m_namedInputs.size()  == 1);
    Q_ASSERT(m_outputs.size() == 1);
    Q_ASSERT(m_outputs[0].volume->toITK().IsNotNull());

    if (!m_inputs.isEmpty())
    {
      Q_ASSERT(m_inputs.size() == 1);
      update = m_outputs[0].volume->toITK()->GetTimeStamp() < m_inputs[0]->toITK()->GetTimeStamp();
    }
  }

  return update;
}

//-----------------------------------------------------------------------------
void FillHolesFilter::run()
{
  Q_ASSERT(m_inputs.size() == 1);

  m_filter = BinaryFillholeFilter::New();
  m_filter->SetInput(m_inputs[0]->toITK());
  m_filter->Update();

  createOutput(0, m_filter->GetOutput());
}
