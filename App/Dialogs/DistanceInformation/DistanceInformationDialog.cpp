/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Extensible.hxx>
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/ItemExtension.hxx>
#include "DistanceInformationDialog.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/Settings.h>
#include <Support/Widgets/TabularReport.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/Spatial.h>
#include <Extensions/Morphological/MorphologicalInformation.h>

// QT
#include <QDebug>

// VTK
#include <vtkDistancePolyDataFilter.h>
#include <vtkPointData.h>

using ESPINA::Core::Analysis::Extensions;
using ESPINA::Core::Extension;
using ESPINA::Core::SegmentationExtension;
using ESPINA::Extensions::MorphologicalInformation;
using ESPINA::GUI::DefaultDialogs;
using namespace ESPINA;

const QString SETTINGS_GROUP = "Raw Information Report";

//----------------------------------------------------------------------------
DistanceInformationDialog::DistanceInformationDialog(SegmentationAdapterList selectedSegmentations,
                                                     DistanceInformationOptionsDialog::Options options,
                                                     Support::Context &context)
: QDialog(DefaultDialogs::defaultParentWidget(), Qt::WindowStaysOnTopHint)
, m_selectedSegmentations{selectedSegmentations}
, m_context              (context)
, m_options              (options)
, m_ERROR_VAL            {-DBL_MAX}
{
  setWindowTitle(tr("Distance Information Report"));
  setWindowIconText(":/espina/Espina.svg");
  setLayout(new QVBoxLayout());

  if (options.maximumDistanceEnabled)
  {
    auto maxDistance = options.maximumDistance;
  }

  auto segmentationsList = context.model()->segmentations();

  for (auto seg1 : selectedSegmentations)
  {
    for (auto seg2 : segmentationsList)
    {
      auto cent = centroidDistance(seg1,seg2.get());
    }
  }

  auto report = new TabularReport(context, this);
  report->setModel(context.model());
  report->setFilter(selectedSegmentations);

  layout()->addWidget(report);
  auto acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(acceptButton, SIGNAL(accepted()),
          this,         SLOT(accept()));
  layout()->addWidget(acceptButton);
}

//----------------------------------------------------------------------------
const Nm DistanceInformationDialog::centroidDistance( SegmentationAdapterPtr first,
                                                      SegmentationAdapterPtr second)
{
  Nm returnValue = m_ERROR_VAL;

  if (!isDistanceCached(first, second, &returnValue))
  {
    auto centroid1 = getCentroid(first);
    auto c1x = centroid1[0];
    auto c1y = centroid1[1];
    auto c1z = centroid1[2];

    auto centroid2 = getCentroid(second);
    auto c2x = centroid2[0];
    auto c2y = centroid2[1];
    auto c2z = centroid2[2];

    returnValue = sqrt(pow(c1x - c2x, 2) + pow(c1y - c2y, 2) + pow(c1z - c2z, 2));
    m_cachedDistances[first][second] = returnValue;
  }

  return returnValue;
}

//----------------------------------------------------------------------------
const bool DistanceInformationDialog::isDistanceCached(SegmentationAdapterPtr first,
                                                        SegmentationAdapterPtr second,
                                                        Nm *distance)
{
  bool returnValue;

  returnValue = m_cachedDistances.contains(second) && m_cachedDistances.value(second).contains(first);
  if (returnValue){
    if (distance != nullptr) *distance = m_cachedDistances[second][first];
  }
  else
  {
    returnValue = m_cachedDistances.contains(first) && m_cachedDistances.value(first).contains(second);
    if (returnValue && distance != nullptr) *distance = m_cachedDistances[first][second];
  }
  return returnValue;
}

//----------------------------------------------------------------------------
const NmVector3 DistanceInformationDialog::getCentroid(SegmentationAdapterPtr seg)
{
  auto segExtensions = seg->extensions();

  if(!segExtensions->hasExtension(MorphologicalInformation::TYPE))
  {
    auto extension = m_context.factory()->createSegmentationExtension(MorphologicalInformation::TYPE);
    segExtensions->add(extension);
  }

  auto morphological = segExtensions->get<MorphologicalInformation>();
  auto keys = morphological->availableInformation();

  NmVector3 returnValue = NmVector3();
  bool isOk;
  Vector3<QString> tagList = {"Centroid X","Centroid Y","Centroid Z"};
  for(int i = 0; i<3; i++)
  {
    auto key = SegmentationExtension::InformationKey( MorphologicalInformation::TYPE,
                                                     tagList[i]);
    auto value_qvariant = morphological->information(key);
    auto value_double = value_qvariant.toDouble(&isOk);
    if(!isOk)
    {
      cerr << "ESPINA::DistanceInformationDialog::getCentroid unexpected variant value";
      exit (1);
    }
    returnValue[i] = value_double;
  }
  return returnValue;
}

//----------------------------------------------------------------------------
const Nm DistanceInformationDialog::meshDistance( SegmentationAdapterPtr first,
                                                  SegmentationAdapterPtr second)
{
  Nm returnValue = m_ERROR_VAL;

  if (!isDistanceCached(first, second, &returnValue))
  {
    auto mesh1 = readLockMesh(first->output());
    auto poly1 = mesh1->mesh();

    auto mesh2 = readLockMesh(second->output());
    auto poly2 = mesh2->mesh();

    auto distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
    distanceFilter->SignedDistanceOff();
    distanceFilter->SetInputData(0, poly1);
    distanceFilter->SetInputData(1, poly2);
    distanceFilter->Update();

    returnValue = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
  }

  return returnValue;
}
