/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "ImageLogicFilter.h"

#include <processing/pqFilter.h>
#include <cache/CachedObjectBuilder.h>
#include <model/EspinaFactory.h>
#include <vtkImageAlgorithm.h>

//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(QList< Segmentation* > input,
				   ImageLogicFilter::Operation op)
: m_args  (new ILFArguments())
, m_filter(NULL)
, m_seg   (NULL)
{
  m_args->setInput(input);
  m_args->setOperation(op);

  run();
}

//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(ModelItem::Arguments args)
: m_args(new ILFArguments(args))
{
  run();
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
  delete m_args;

  if (m_filter)
    delete m_filter;

  if (m_seg)
    delete m_seg;

}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  QString segId = id() + "_0";
  if ((m_filter = cob->loadFile(segId)) == NULL)
  {
    pqFilter::Arguments logic;
    logic << pqFilter::Argument("Input",pqFilter::Argument::INPUT, m_args->value(ILFArguments::INPUT));
    logic << pqFilter::Argument("Operation",pqFilter::Argument::INTVECT, m_args->value(ILFArguments::OPERATION));
    m_filter = cob->createFilter("filters","ImageLogicFilter", logic);
    Q_ASSERT(m_filter->getNumberOfData() == 1);
  }

  m_filter->algorithm()->Update();

  m_seg = EspinaFactory::instance()->createSegmentation(this, 0);
}

//-----------------------------------------------------------------------------
QString ImageLogicFilter::id() const
{
  return m_args->hash();
}

//-----------------------------------------------------------------------------
QVariant ImageLogicFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return ILF;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString ImageLogicFilter::serialize() const
{
  return m_args->serialize();
}

//-----------------------------------------------------------------------------
int ImageLogicFilter::numProducts() const
{
  return m_filter?1:0;
}

//-----------------------------------------------------------------------------
Segmentation* ImageLogicFilter::product(int index) const
{
  Q_ASSERT(m_filter->getNumberOfData() > index);
  return m_seg;
}

//-----------------------------------------------------------------------------
pqData ImageLogicFilter::preview()
{
  Q_ASSERT(false);
  return pqData(NULL, -1);
}

//-----------------------------------------------------------------------------
QWidget* ImageLogicFilter::createConfigurationWidget()
{
  return NULL;
}

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ImageLogicFilter::ILFArguments::INPUT = ArgumentId("INPUT", true);
const ArgumentId ImageLogicFilter::ILFArguments::OPERATION = ArgumentId("Operation", true);

//-----------------------------------------------------------------------------
ImageLogicFilter::ILFArguments::ILFArguments(const Arguments args)
: Arguments(args)
{
  QStringList input = args[INPUT].split(",");
  //TODO: Recover segmentation pointers

  m_operation = Operation(args[OPERATION].toInt());

}
