/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


// EspINA
#include "SplitFilter.h"
#include <Core/EspinaVolume.h>

// ITK
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <vtkImageStencilData.h>

// VTK
#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageToImageStencil.h>
#include <vtkMath.h>

// Qt
#include <QDir>
#include <QDebug>

using namespace EspINA;

const QString SplitFilter::INPUTLINK = "Segmentation";

//-----------------------------------------------------------------------------
SplitFilter::SplitFilter(NamedInputs inputs,
                         Arguments   args,
                         FilterType  type)
: SegmentationFilter(inputs, args, type)
, m_stencil(NULL)
, m_ignoreCurrentOutputs(false)
{
}

//-----------------------------------------------------------------------------
SplitFilter::~SplitFilter()
{
}

//-----------------------------------------------------------------------------
bool SplitFilter::needUpdate(OutputId oId) const
{
  return Filter::needUpdate(oId);
}

//-----------------------------------------------------------------------------
void SplitFilter::run()
{
  for (int i = 0; i < 2; ++i)
    run(i);
}

//-----------------------------------------------------------------------------
void SplitFilter::run(Filter::OutputId oId)
{
  //qDebug() << "Split Run" << m_cacheId;
  Q_ASSERT(0 == oId || 1 == oId);
  Q_ASSERT(m_inputs.size() == 1);

  EspinaVolume::Pointer input = m_inputs[0];

  // if you want to run the filter you must have an stencil
  if (NULL == m_stencil.GetPointer())
    Q_ASSERT(fetchCacheStencil());

  EspinaRegion region = input->espinaRegion();
  itkVolumeType::SpacingType spacing = input->toITK()->GetSpacing();

  if (m_volumes[0].get() == NULL || ignoreCurrentOutputs())
  {
    for(int i=0; i < 2; i++)
      m_volumes[i] = SegmentationVolume::Pointer(new SegmentationVolume(region, spacing));

    itkVolumeConstIterator it = input      ->iterator(region);
    itkVolumeIterator     ot1 = m_volumes[0] ->iterator(region);
    itkVolumeIterator     ot2 = m_volumes[1] ->iterator(region);

    it .GoToBegin();
    ot1.GoToBegin();
    ot2.GoToBegin();

    bool isEmpty1 = true;
    bool isEmpty2 = true;

    int shift[3]; // Stencil origin differ from creation to fetch
    for (int i = 0; i < 3; ++i)
      shift[i] = vtkMath::Round(m_stencil->GetOrigin()[i] / m_stencil->GetSpacing()[i]);

    for(; !it.IsAtEnd(); ++it, ++ot1, ++ot2)
    {
      itkVolumeType::IndexType index = ot1.GetIndex();
      if (m_stencil->IsInside(index[0] - shift[0], index[1]- shift[1], index[2]- shift[2]))
      {
        ot1.Set(it.Value());
        if (isEmpty1)
          isEmpty1 = ot1.Get() != SEG_VOXEL_VALUE;
      }
      else
      {
        ot2.Set(it.Value());
        if (isEmpty2)
          isEmpty2 = ot2.Get() != SEG_VOXEL_VALUE;
      }
    }

    if (!isEmpty1 && !isEmpty2)
    {
      for (int i = 0; i < 2; i++)
        m_volumes[i]->fitToContent();

      m_ignoreCurrentOutputs = false;

      emit modified(this);
    }
  }

  if (!ignoreCurrentOutputs())
    createOutput(oId, m_volumes[oId]);
}

//-----------------------------------------------------------------------------
bool SplitFilter::fetchCacheStencil()
{
  if (m_ignoreCurrentOutputs)
    return false;

  bool returnVal = false;

  if (m_cacheDir.exists(QString().number(m_cacheId) + QString("-Stencil.vti")))
  {
    QString fileName = m_cacheDir.absolutePath() + QDir::separator() + QString().number(m_cacheId) + QString("-Stencil.vti");
    vtkSmartPointer<vtkGenericDataObjectReader> stencilReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    stencilReader->SetFileName(fileName.toStdString().c_str());
    stencilReader->ReadAllFieldsOn();
    stencilReader->Update();

    vtkSmartPointer<vtkImageToImageStencil> convert = vtkSmartPointer<vtkImageToImageStencil>::New();
    convert->SetInputConnection(stencilReader->GetOutputPort());
    convert->ThresholdBetween(1,1);
    convert->Update();

    m_stencil = vtkSmartPointer<vtkImageStencilData>(convert->GetOutput());

    returnVal = true;
  }

  return returnVal;
}

//-----------------------------------------------------------------------------
bool SplitFilter::dumpSnapshot(QList<QPair<QString, QByteArray> > &fileList)
{
  bool returnVal = false;

  if (m_traceable)
  {
    // check if the stencil is in the cache and hasn't been loaded (because a run() wasn't needed)
    if (NULL == m_stencil.GetPointer())
      fetchCacheStencil();

    if (m_stencil != NULL)
    {
      vtkSmartPointer<vtkImageStencilToImage> convert = vtkSmartPointer<vtkImageStencilToImage>::New();
      convert->SetInputConnection(m_stencil->GetProducerPort());
      convert->SetInsideValue(1);
      convert->SetOutsideValue(0);
      convert->SetOutputScalarTypeToUnsignedChar();
      convert->Update();

      vtkSmartPointer<vtkGenericDataObjectWriter> stencilWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
      stencilWriter->SetInputConnection(convert->GetOutputPort());
      stencilWriter->SetFileTypeToBinary();
      stencilWriter->SetWriteToOutputString(true);
      stencilWriter->Write();

      QByteArray stencilArray(stencilWriter->GetOutputString(), stencilWriter->GetOutputStringLength());
      QPair<QString, QByteArray> stencilEntry(this->id() + QString("-Stencil.vti"), stencilArray);

      fileList << stencilEntry;

      returnVal = true;
    }
  }

  return (SegmentationFilter::dumpSnapshot(fileList) || returnVal);
}
