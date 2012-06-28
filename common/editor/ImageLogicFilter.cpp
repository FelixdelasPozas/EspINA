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
#include <QDebug>

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
, m_pad1(PadFilterType::New())
, m_pad2(PadFilterType::New())
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
  Q_ASSERT(m_inputs.size() > 1);

  qDebug() << "Compute output boundaries";
  int inExt1[6], inExt2[6], outExt[6];

  VolumeExtent(m_inputs[0], inExt1);
  VolumeExtent(m_inputs[1], inExt2);

  for (int i=0; i<3; i++)
  {
    int lower = 2*i;
    int upper = lower + 1;
    outExt[lower] = std::min(inExt1[lower], inExt2[lower]);
    outExt[upper] = std::max(inExt1[upper], inExt2[upper]);
  }

  qDebug() << "Expand Input 1";
  EspinaVolume::SizeType lowerPad1;
  lowerPad1[0] = inExt1[0] - outExt[0];
  lowerPad1[1] = inExt1[2] - outExt[2];
  lowerPad1[2] = inExt1[4] - outExt[4];
  EspinaVolume::SizeType upperPad1;
  upperPad1[0] = outExt[1] - inExt1[1];
  upperPad1[1] = outExt[3] - inExt1[3];
  upperPad1[2] = outExt[5] - inExt1[5];
  m_pad1->SetInput(m_inputs[0]);
  m_pad1->SetPadLowerBound(lowerPad1);
  m_pad1->SetPadUpperBound(upperPad1);
  m_pad1->SetConstant(0);
  m_pad1->Update();
  //Debug only
  EspinaVolume::RegionType reg1 = m_pad1->GetOutput()->GetLargestPossibleRegion();

  qDebug() << "Expand Input 2";
  EspinaVolume::SizeType lowerPad2;
  lowerPad2[0] = inExt2[0] - outExt[0];
  lowerPad2[1] = inExt2[2] - outExt[2];
  lowerPad2[2] = inExt2[4] - outExt[4];
  EspinaVolume::SizeType upperPad2;
  upperPad2[0] = outExt[1] - inExt2[1];
  upperPad2[1] = outExt[3] - inExt2[3];
  upperPad2[2] = outExt[5] - inExt2[5];
  m_pad2->SetInput(m_inputs[0]);
  m_pad2->SetPadLowerBound(lowerPad2);
  m_pad2->SetPadUpperBound(upperPad2);
  m_pad2->SetConstant(0);
  m_pad2->Update();
  //Debug only
  EspinaVolume::RegionType reg2 = m_pad2->GetOutput()->GetLargestPossibleRegion();

  m_orFilter->SetInput1(m_pad1->GetOutput());
  m_orFilter->SetInput2(m_pad2->GetOutput());
  m_orFilter->Update();

  m_volume = m_orFilter->GetOutput();
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