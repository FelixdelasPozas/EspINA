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


#include "CODEFilter.h"
#include <cache/CachedObjectBuilder.h>
#include <model/EspinaFactory.h>
#include <vtkImageAlgorithm.h>

//-----------------------------------------------------------------------------
CODEFilter::CODEFilter(Segmentation* seg, CODEFilter::Operation op, unsigned int radius)
: m_args(new CODEArguments())
, m_filter(NULL)
, m_seg(NULL)
{
  m_args->setInput(seg);
  m_args->setOperation(op);
  m_args->setRadius(radius);

  run();
}

//-----------------------------------------------------------------------------
CODEFilter::CODEFilter(ModelItem::Arguments args)
: m_args(new CODEArguments(args))
{
  run();
}

//-----------------------------------------------------------------------------
CODEFilter::~CODEFilter()
{
  delete m_args;

  if (m_seg)
    delete m_seg;

  if (m_filter)
    delete m_filter;
}

//-----------------------------------------------------------------------------
void CODEFilter::run()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  QString segId = id() + "_0";
  if ((m_filter = cob->loadFile(segId)) == NULL)
  {
    pqFilter::Arguments args;
    args << pqFilter::Argument("Input",pqFilter::Argument::INPUT, m_args->value(CODEArguments::INPUT));
    args << pqFilter::Argument("Radius",pqFilter::Argument::INTVECT, m_args->value(CODEArguments::RADIUS));
    args << pqFilter::Argument("Operation",pqFilter::Argument::UNKOWN, m_args->value(CODEArguments::OPERATION));
    switch (m_args->operation())
    {
      case CLOSE:
	m_filter = cob->createFilter("filters","ClosingImageFilter", args);
	break;
      case OPEN:
	m_filter = cob->createFilter("filters","OpeningImageFilter", args);
	break;
      case DILATE:
	m_filter = cob->createFilter("filters","DilateImageFilter", args);
	break;
      case ERODE:
	m_filter = cob->createFilter("filters","ErodeImageFilter", args);
	break;
    }
    Q_ASSERT(m_filter->getNumberOfData() == 1);
  }
  m_filter->algorithm()->Update();

  m_seg = EspinaFactory::instance()->createSegmentation(this, 0);
}

//-----------------------------------------------------------------------------
QString CODEFilter::id() const
{
  return m_args->hash();
}

//-----------------------------------------------------------------------------
QVariant CODEFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return CODE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString CODEFilter::serialize() const
{
  return m_args->serialize();
}

//-----------------------------------------------------------------------------
int CODEFilter::numProducts() const
{
  return m_filter?1:0;
}

//-----------------------------------------------------------------------------
Segmentation* CODEFilter::product(int index) const
{
  Q_ASSERT(m_filter->getNumberOfData() > index);
  return m_seg;
}

//-----------------------------------------------------------------------------
pqData CODEFilter::preview()
{
  Q_ASSERT(false);
  return pqData(NULL, -1);
}

//-----------------------------------------------------------------------------
QWidget* CODEFilter::createConfigurationWidget()
{
  return NULL;
}

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId CODEFilter::CODEArguments::INPUT = ArgumentId("INPUT", true);
const ArgumentId CODEFilter::CODEArguments::OPERATION = ArgumentId("Operation", true);
const ArgumentId CODEFilter::CODEArguments::RADIUS = ArgumentId("Radius", true);

//-----------------------------------------------------------------------------
CODEFilter::CODEArguments::CODEArguments(const Arguments args)
: Arguments(args)
{
  //TODO: Recover segmentation pointers

  m_operation = Operation(args[OPERATION].toInt());
}