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

const unsigned int LABEL_VALUE = 255;

//-----------------------------------------------------------------------------
ClosingFilter::ClosingFilter(Segmentation* seg, unsigned int radius)
: m_args(new ClosingArguments())
, m_input(seg)
, m_volume(NULL)
{
  m_args->setInput(seg);
  m_args->setRadius(radius);

  update();
}

//-----------------------------------------------------------------------------
ClosingFilter::ClosingFilter(ModelItem::Arguments args)
: m_args(new ClosingArguments(args))
, m_input(NULL)
, m_volume(NULL)
{
  Q_ASSERT(false);
  run();
}

//-----------------------------------------------------------------------------
ClosingFilter::~ClosingFilter()
{
  delete m_args;
}

//-----------------------------------------------------------------------------
void ClosingFilter::run()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  qDebug() << "Compute Image Closing";
  StructuringElementType ball;
  ball.SetRadius(m_args->radius());
  ball.CreateStructuringElement();

  m_filter = FilterType::New();
  m_filter->SetInput(m_input->volume());
  m_filter->SetKernel(ball);
  m_filter->SetForegroundValue(LABEL_VALUE);
  m_filter->Update();
  QApplication::restoreOverrideCursor();

  m_volume = m_filter->GetOutput();
}

//-----------------------------------------------------------------------------
QString ClosingFilter::id() const
{
  return m_args->hash();
}

//-----------------------------------------------------------------------------
QVariant ClosingFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return Closing;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString ClosingFilter::serialize() const
{
  return m_args->serialize();
}

//-----------------------------------------------------------------------------
int ClosingFilter::numberOutputs() const
{
 return m_volume?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* ClosingFilter::output(int i) const
{
  if (m_volume && i == 0)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
QWidget* ClosingFilter::createConfigurationWidget()
{
  return NULL;
}

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ClosingFilter::ClosingArguments::INPUT = ArgumentId("INPUT", true);
const ArgumentId ClosingFilter::ClosingArguments::RADIUS = ArgumentId("Radius", true);

//-----------------------------------------------------------------------------
ClosingFilter::ClosingArguments::ClosingArguments(const Arguments args)
: Arguments(args)
{
  //TODO: Recover segmentation pointers
}