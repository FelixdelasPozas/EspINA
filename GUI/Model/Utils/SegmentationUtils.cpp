/*

    Copyright (C) 2016 Felix de las Pozas Alvarez<fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "SegmentationUtils.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/ModelAdapter.h>

// VTK
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
SegmentationAdapterPtr ESPINA::GUI::Model::Utils::segmentationPtr(ItemAdapterPtr item)
{
  return dynamic_cast<SegmentationAdapterPtr>(item);
}

//------------------------------------------------------------------------
ConstSegmentationAdapterPtr ESPINA::GUI::Model::Utils::segmentationPtr(ConstItemAdapterPtr item)
{
  return dynamic_cast<ConstSegmentationAdapterPtr>(item);
}

//------------------------------------------------------------------------
bool ESPINA::GUI::Model::Utils::isSegmentation(ItemAdapterPtr item)
{
  return ItemAdapter::Type::SEGMENTATION == item->type();
}

//------------------------------------------------------------------------
const QString ESPINA::GUI::Model::Utils::categoricalName(SegmentationAdapterSPtr segmentation)
{
  return categoricalName(segmentation.get());
}

//------------------------------------------------------------------------
const QString ESPINA::GUI::Model::Utils::categoricalName(SegmentationAdapterPtr segmentation)
{
  auto name = QString("%1 %2").arg(segmentation->category() ? segmentation->category()->name() : "Unknown Category")
                              .arg(segmentation->number());

  return name;
}

//------------------------------------------------------------------------
ConnectionList ESPINA::GUI::Model::Utils::connections(vtkSmartPointer<vtkPolyData> polyData, const ModelAdapterSPtr model)
{
  ConnectionList connections;
  auto terminal = vtkDoubleArray::SafeDownCast(polyData->GetPointData()->GetAbstractArray("TerminalNodes"));

  if(terminal)
  {
    for(int i = 0; i < terminal->GetNumberOfTuples(); ++i)
    {
      double value[3];
      terminal->GetTuple(i, value);

      NmVector3 point{value};

      for(auto candidate: model->contains(point))
      {
        auto seg = std::dynamic_pointer_cast<SegmentationAdapter>(candidate);

        if(!seg || !seg->category()->classificationName().startsWith("Synapse", Qt::CaseInsensitive) || !hasVolumetricData(seg->output())) continue;

        if(isSegmentationVoxel(readLockVolume(seg->output()), point))
        {
          Connection connection;
          connection.item1 = nullptr;
          connection.item2 = seg;
          connection.point = point;

          connections << connection;
        }
      }
    }
  }

  return connections;
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ESPINA::GUI::Model::Utils::axonOf(const SegmentationAdapterPtr synapse)
{
  SegmentationAdapterSPtr result = nullptr;

  if(synapse)
  {
    auto model = synapse->model();
    auto synapseSPtr = model->smartPointer(synapse);
    if(synapseSPtr)
    {
      for(auto connection: model->connections(synapseSPtr))
      {
        if(connection.item2->category()->classificationName().startsWith("Axon", Qt::CaseInsensitive))
        {
          result = connection.item2;
          break;
        }
      }
    }
  }

  return result;
}

//------------------------------------------------------------------------
SegmentationAdapterSPtr ESPINA::GUI::Model::Utils::dendriteOf(const SegmentationAdapterPtr synapse)
{
  SegmentationAdapterSPtr result = nullptr;

  if(synapse)
  {
    auto model = synapse->model();
    auto synapseSPtr = model->smartPointer(synapse);
    if(synapseSPtr)
    {
      for(auto connection: model->connections(synapseSPtr))
      {
        if(connection.item2->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive))
        {
          result = connection.item2;
          break;
        }
      }
    }
  }

  return result;
}

//------------------------------------------------------------------------
const QString ESPINA::GUI::Model::Utils::segmentationListNames(const SegmentationAdapterList& list)
{
  QString result;
  for(auto segmentation: list)
  {
    if(segmentation != list.first())
    {
      result += (segmentation != list.last() ? ", ":" and ");
    }

    result += "'" + segmentation->data(Qt::DisplayRole).toString() + "'";
  }

  return result;
}

//------------------------------------------------------------------------
const QString ESPINA::GUI::Model::Utils::segmentationListNames(const SegmentationAdapterSList& list)
{
  return segmentationListNames(rawList(list));
}
