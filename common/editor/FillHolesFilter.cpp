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
#include <QApplication>

const QString FillHolesFilter::TYPE = "EditorToolBar::FillHolesFilter";
const QString FillHolesFilter::INPUTLINK = "Input";

//-----------------------------------------------------------------------------
FillHolesFilter::FillHolesFilter(Filter::NamedInputs inputs,
                                 ModelItem::Arguments args)
: Filter(inputs, args)
{
}

//-----------------------------------------------------------------------------
FillHolesFilter::~FillHolesFilter()
{
}

//-----------------------------------------------------------------------------
QVariant FillHolesFilter::data(int role) const
{
  if (Qt::DisplayRole == role)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
bool FillHolesFilter::needUpdate() const
{
  bool update = true;

  if (!m_outputs.isEmpty())
  {
    Q_ASSERT(m_inputs.size()  == 1);
    Q_ASSERT(m_outputs.size() == 1);
    Q_ASSERT(m_outputs[0].volume.IsNotNull());

    update = m_outputs[0].volume->GetTimeStamp() < m_inputs[0]->GetTimeStamp();
  }

  return update;
}

//-----------------------------------------------------------------------------
void FillHolesFilter::run()
{
  // TODO 2012-11-20: Quitar los override cursor de los filtros, deberia ser
  // responsabilidad de la aplicacion el modificarlo
  QApplication::setOverrideCursor(Qt::WaitCursor);
  Q_ASSERT(m_inputs.size() == 1);

  m_filter = FilterType::New();
  m_filter->SetInput(m_inputs[0]);
  m_filter->Update();
  QApplication::restoreOverrideCursor();

  m_outputs.clear();
  m_outputs << FilterOutput(this, 0, m_filter->GetOutput());
//   if (m_outputs.isEmpty())
//     m_outputs << output(this, 0, m_filter->GetOutput());
//   else if (m_outputs.size() == 1)
//   {
//     m_outputs[0].volume   = m_filter->GetOutput();
//     m_outputs[0].isCached = false;
//     ...
//   }
//   else
//     Q_ASSERT(false);
}
