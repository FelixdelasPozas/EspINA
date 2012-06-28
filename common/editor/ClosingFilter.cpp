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

#include <model/EspinaFactory.h>

#include <QDebug>
#include <QApplication>

const QString ClosingFilter::TYPE = "EditorToolBar::ClosingFilter";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ClosingFilter::RADIUS = ArgumentId("Radius", true);

const unsigned int LABEL_VALUE = 255;

ClosingFilter::ClosingFilter(Filter::NamedInputs inputs,
                             ModelItem::Arguments args)
: m_inputs(inputs)
, m_args(args)
, m_input(NULL)
, m_volume(NULL)
{
//   qDebug() << TYPE << "arguments" << m_args;
}


//-----------------------------------------------------------------------------
ClosingFilter::~ClosingFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void ClosingFilter::run()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QStringList input = m_args[INPUTS].split("_");
  m_input = m_inputs[input[0]]->output(input[1].toUInt());

  qDebug() << "Compute Image Closing";
  StructuringElementType ball;
  ball.SetRadius(m_args.radius());
  ball.CreateStructuringElement();

  m_filter = FilterType::New();
  m_filter->SetInput(m_input);
  m_filter->SetKernel(ball);
  m_filter->SetForegroundValue(LABEL_VALUE);
  m_filter->Update();
  QApplication::restoreOverrideCursor();

  m_volume = m_filter->GetOutput();
}

//-----------------------------------------------------------------------------
QString ClosingFilter::id() const
{
  return m_args[ID];
}

//-----------------------------------------------------------------------------
QVariant ClosingFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString ClosingFilter::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
int ClosingFilter::numberOutputs() const
{
  return m_volume?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* ClosingFilter::output(OutputNumber i) const
{
  if (m_volume && i == 0)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
bool ClosingFilter::prefetchFilter()
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
QWidget* ClosingFilter::createConfigurationWidget()
{
  return NULL;
}
