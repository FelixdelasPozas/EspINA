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

const unsigned int LABEL_VALUE = 255;

//-----------------------------------------------------------------------------
ErodeFilter::ErodeFilter(Segmentation* seg, unsigned int radius)
: m_args(new ErodeArguments())
, m_input(seg)
, m_volume(NULL)
{
  m_args->setInput(seg);
  m_args->setRadius(radius);

  update();
}

//-----------------------------------------------------------------------------
ErodeFilter::ErodeFilter(ModelItem::Arguments args)
: m_args(new ErodeArguments(args))
, m_input(NULL)
, m_volume(NULL)
{
  Q_ASSERT(false);
  run();
}

//-----------------------------------------------------------------------------
ErodeFilter::~ErodeFilter()
{
  delete m_args;
}

//-----------------------------------------------------------------------------
void ErodeFilter::run()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  qDebug() << "Compute Image Erode";
  StructuringElementType ball;
  ball.SetRadius(m_args->radius());
  ball.CreateStructuringElement();

  m_filter = bmcifType::New();
  m_filter->SetInput(m_input->volume());
  m_filter->SetKernel(ball);
  m_filter->SetObjectValue(LABEL_VALUE);
  m_filter->Update();
  QApplication::restoreOverrideCursor();

  m_volume = m_filter->GetOutput();
}

//-----------------------------------------------------------------------------
QString ErodeFilter::id() const
{
  return m_args->hash();
}

//-----------------------------------------------------------------------------
QVariant ErodeFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return Erode;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString ErodeFilter::serialize() const
{
  return m_args->serialize();
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
QWidget* ErodeFilter::createConfigurationWidget()
{
  return NULL;
}

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ErodeFilter::ErodeArguments::INPUT = ArgumentId("INPUT", true);
const ArgumentId ErodeFilter::ErodeArguments::RADIUS = ArgumentId("Radius", true);

//-----------------------------------------------------------------------------
ErodeFilter::ErodeArguments::ErodeArguments(const Arguments args)
: Arguments(args)
{
  //TODO: Recover segmentation pointers
}