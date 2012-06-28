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

#include <model/EspinaFactory.h>
#include <vtkImageAlgorithm.h>
#include <QApplication>

const QString ImageLogicFilter::TYPE = "EditorToolBar::ImageLogicFilter";

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ImageLogicFilter::OPERATION = ArgumentId("Operation", true);


//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(Filter::NamedInputs inputs,
                                   ModelItem::Arguments args)
: Filter(inputs, args)
, m_param(m_args)
, m_volume(NULL)
, m_orFilter(OrFilterType::New())
{
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
int ImageLogicFilter::numberOutputs() const
{
  return m_volume?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* ImageLogicFilter::output(OutputNumber i) const
{
  if (m_volume && i == 0)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::prefetchFilter()
{
return Filter::prefetchFilter();
}

//-----------------------------------------------------------------------------
QVariant ImageLogicFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}