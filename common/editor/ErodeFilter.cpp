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

#include <model/EspinaFactory.h>

#include <QDebug>
#include <QApplication>

const QString ErodeFilter::TYPE = "EditorToolBar::ErodeFilter";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ErodeFilter::RADIUS = ArgumentId("Radius", true);

const unsigned int LABEL_VALUE = 255;

ErodeFilter::ErodeFilter(Filter::NamedInputs inputs,
                             ModelItem::Arguments args)
: Filter(inputs, args)
, m_params(m_args)
, m_input(NULL)
, m_volume(NULL)
{
//   qDebug() << TYPE << "arguments" << m_args;
}


//-----------------------------------------------------------------------------
ErodeFilter::~ErodeFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
bool ErodeFilter::needUpdate() const
{
  return m_volume == NULL;
}

//-----------------------------------------------------------------------------
void ErodeFilter::run()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  Q_ASSERT(m_inputs.size() == 1);
  m_input = m_inputs.first();

  qDebug() << "Compute Image Erode";
  StructuringElementType ball;
  ball.SetRadius(m_params.radius());
  ball.CreateStructuringElement();

  m_filter = FilterType::New();
  m_filter->SetInput(m_input);
  m_filter->SetKernel(ball);
  m_filter->SetObjectValue(LABEL_VALUE);
  m_filter->Update();
  QApplication::restoreOverrideCursor();

  m_volume = m_filter->GetOutput();
}

//-----------------------------------------------------------------------------
QVariant ErodeFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}


//-----------------------------------------------------------------------------
int ErodeFilter::numberOutputs() const
{
  return m_volume?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* ErodeFilter::output(OutputNumber i) const
{
  if (m_volume && i == 0)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
bool ErodeFilter::prefetchFilter()
{
  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_volume = m_cachedFilter->GetOutput();
    emit modified(this);
    return true;
  }

  return false;
}