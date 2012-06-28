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


#include "DilateFilter.h"

#include <model/EspinaFactory.h>

#include <QDebug>
#include <QApplication>

const QString DilateFilter::TYPE = "EditorToolBar::DilateFilter";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId DilateFilter::RADIUS = ArgumentId("Radius", true);

const unsigned int LABEL_VALUE = 255;

DilateFilter::DilateFilter(Filter::NamedInputs inputs,
                             ModelItem::Arguments args)
: m_inputs(inputs)
, m_args(args)
, m_input(NULL)
, m_volume(NULL)
{
  qDebug() << TYPE << "arguments" << m_args;
}


//-----------------------------------------------------------------------------
DilateFilter::~DilateFilter()
{
  qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void DilateFilter::run()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QStringList input = m_args[INPUTS].split("_");
  qDebug() << m_inputs[input[0]]->data(Qt::DisplayRole).toString();
  m_input = m_inputs[input[0]]->output(input[1].toUInt());
  m_input->Update();

  unsigned int radius = m_args.radius();
  qDebug() << "Compute Image Dilate" << radius;
  StructuringElementType ball;
  ball.SetRadius(radius);
  ball.CreateStructuringElement();

  m_filter = FilterType::New();
  m_filter->SetInput(m_input);
  m_filter->SetKernel(ball);
  m_filter->SetObjectValue(LABEL_VALUE);
  m_filter->SetReleaseDataFlag(false);
  m_filter->Update();
  QApplication::restoreOverrideCursor();

  m_volume = m_filter->GetOutput();
}

//-----------------------------------------------------------------------------
QString DilateFilter::id() const
{
  return m_args[ID];
}

//-----------------------------------------------------------------------------
QVariant DilateFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString DilateFilter::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
int DilateFilter::numberOutputs() const
{
  return m_volume?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* DilateFilter::output(OutputNumber i) const
{
  if (m_volume && i == 0)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
bool DilateFilter::prefetchFilter()
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

//-----------------------------------------------------------------------------
QWidget* DilateFilter::createConfigurationWidget()
{
  return NULL;
}
