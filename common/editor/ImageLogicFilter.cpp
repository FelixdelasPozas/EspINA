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
, m_filter(FilterType::New())
// , m_pad1(PadFilterType::New())
// , m_pad2(PadFilterType::New())
// , m_orFilter(OrFilterType::New())
{
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
}

//-----------------------------------------------------------------------------
QVariant ImageLogicFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
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
    return m_filter->GetOutput();

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::prefetchFilter()
{
  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_volume = m_cachedFilter->GetOutput();
    m_filter->SetInput(m_volume);
    m_filter->Update();
    emit modified(this);
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate() const
{
  return m_volume.IsNull();
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run() //TODO: Parallelize
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  Q_ASSERT(m_inputs.size() > 1);

  m_inputExtents.clear();

  qDebug() << "Compute output boundaries";
  int *inExt1 = new int[6];
  int *inExt2 = new int[6];

  m_inputExtents << inExt1 << inExt2;

  VolumeExtent(m_inputs[0], inExt1);
  VolumeExtent(m_inputs[1], inExt2);

  EspinaVolume::PointType origin1 = m_inputs[0]->GetOrigin();
  EspinaVolume::PointType origin2 = m_inputs[1]->GetOrigin();
  EspinaVolume::PointType outOrigin;

  Q_ASSERT(m_inputs[0]->GetSpacing() == m_inputs[1]->GetSpacing());

  for (int i=0; i<3; i++)
  {
    int lower = 2*i;
    int upper = lower + 1;
    m_outputExtent[lower] = std::min(inExt1[lower], inExt2[lower]);
    m_outputExtent[upper] = std::max(inExt1[upper], inExt2[upper]);
//     outOrigin[i] = std::min(origin1[i], origin2[i]);
  }

  m_volume = EspinaVolume::New();
  EspinaVolume::RegionType buffer = ExtentToRegion(m_outputExtent);
  m_volume->SetRegions(buffer);
  m_volume->SetSpacing(m_inputs[0]->GetSpacing());
  m_volume->SetOrigin(outOrigin);
  m_volume->Allocate();

  switch (m_param.operation())
  {
    case ADDITION:
      addition();
      break;
    case SUBSTRACTION:
      substraction();
      break;
    default:
      Q_ASSERT(false);
  };

  m_filter->SetInput(m_volume);
  m_filter->Update();

  QApplication::restoreOverrideCursor();
}

// NOTE: Alternative implementation using itk logic image filters (not working)
// //-----------------------------------------------------------------------------
// void ImageLogicFilter::run()
// {
//   QApplication::setOverrideCursor(Qt::WaitCursor);
//   Q_ASSERT(m_inputs.size() > 1);
// 
//   qDebug() << "Compute output boundaries";
//   int inExt1[6], inExt2[6], m_outputExtent[6];
// 
//   VolumeExtent(m_inputs[0], inExt1);
//   VolumeExtent(m_inputs[1], inExt2);
// 
//   for (int i=0; i<3; i++)
//   {
//     int lower = 2*i;
//     int upper = lower + 1;
//     m_outputExtent[lower] = std::min(inExt1[lower], inExt2[lower]);
//     m_outputExtent[upper] = std::max(inExt1[upper], inExt2[upper]);
//   }
// 
// 
//   qDebug() << "Expand Input 1";
//   EspinaVolume::SizeType lowerPad1;
//   lowerPad1[0] = inExt1[0] - m_outputExtent[0];
//   lowerPad1[1] = inExt1[2] - m_outputExtent[2];
//   lowerPad1[2] = inExt1[4] - m_outputExtent[4];
//   EspinaVolume::SizeType upperPad1;
//   upperPad1[0] = m_outputExtent[1] - inExt1[1];
//   upperPad1[1] = m_outputExtent[3] - inExt1[3];
//   upperPad1[2] = m_outputExtent[5] - inExt1[5];
//   m_pad1->SetInput(m_inputs[0]);
//   m_pad1->SetPadLowerBound(lowerPad1);
//   m_pad1->SetPadUpperBound(upperPad1);
//   m_pad1->SetConstant(0);
//   m_pad1->Update();
//   //Debug only
//   EspinaVolume::RegionType reg1 = m_pad1->GetOutput()->GetLargestPossibleRegion();
// 
//   qDebug() << "Expand Input 2";
//   EspinaVolume::SizeType lowerPad2;
//   lowerPad2[0] = inExt2[0] - m_outputExtent[0];
//   lowerPad2[1] = inExt2[2] - m_outputExtent[2];
//   lowerPad2[2] = inExt2[4] - m_outputExtent[4];
//   EspinaVolume::SizeType upperPad2;
//   upperPad2[0] = m_outputExtent[1] - inExt2[1];
//   upperPad2[1] = m_outputExtent[3] - inExt2[3];
//   upperPad2[2] = m_outputExtent[5] - inExt2[5];
//   m_pad2->SetInput(m_inputs[0]);
//   m_pad2->SetPadLowerBound(lowerPad2);
//   m_pad2->SetPadUpperBound(upperPad2);
//   m_pad2->SetConstant(0);
//   m_pad2->Update();
//   //Debug only
//   EspinaVolume::RegionType reg2 = m_pad2->GetOutput()->GetLargestPossibleRegion();
// 
//   m_orFilter->SetInput1(m_pad1->GetOutput());
//   m_orFilter->SetInput2(m_pad2->GetOutput());
//   m_orFilter->Update();
// 
//   m_volume = m_orFilter->GetOutput();
//   QApplication::restoreOverrideCursor();
// }

//-----------------------------------------------------------------------------
void ImageLogicFilter::addition()
{
  unsigned char *input1Ptr = static_cast<unsigned char *>(m_inputs[0]->GetBufferPointer());
  unsigned char *input2Ptr = static_cast<unsigned char *>(m_inputs[1]->GetBufferPointer());
  unsigned char *outputPtr = static_cast<unsigned char *>(m_volume->GetBufferPointer());

  for (int z = m_outputExtent[4]; z <= m_outputExtent[5]; z++)
  {
    for (int y = m_outputExtent[2]; y <= m_outputExtent[3]; y++)
    {
      for (int x = m_outputExtent[0]; x <= m_outputExtent[1]; x++)
      {
        if (isExtentPixel(x, y, z, m_inputExtents[0]))
        {
          if (isExtentPixel(x, y, z, m_inputExtents[1]))
          {
            *outputPtr = (*input1Ptr) | (*input2Ptr);
            input2Ptr++;
          }else
            *outputPtr = *input1Ptr;

          input1Ptr++;
        } else if (isExtentPixel(x, y, z, m_inputExtents[1]))
        {
          *outputPtr = *input2Ptr;
          input2Ptr++;
        } else
          *outputPtr = 0;

        outputPtr++;
      }
    }
  }
}

void ImageLogicFilter::substraction()
{
  unsigned char *input1Ptr = static_cast<unsigned char *>(m_inputs[0]->GetBufferPointer());
  unsigned char *input2Ptr = static_cast<unsigned char *>(m_inputs[1]->GetBufferPointer());
  unsigned char *outputPtr = static_cast<unsigned char *>(m_volume->GetBufferPointer());

  for (int z = m_outputExtent[4]; z <= m_outputExtent[5]; z++)
  {
    for (int y = m_outputExtent[2]; y <= m_outputExtent[3]; y++)
    {
      for (int x = m_outputExtent[0]; x <= m_outputExtent[1]; x++)
      {
        *outputPtr = 0;
        if (isExtentPixel(x, y, z, m_inputExtents[0]))
        {
          *outputPtr = *input1Ptr;
          input1Ptr++;
        }
        if (isExtentPixel(x, y, z, m_inputExtents[1]))
        {
          if (*outputPtr)
            *outputPtr ^= *input2Ptr;
          input2Ptr++;
        }
        outputPtr++;
      }
    }
  }
}
