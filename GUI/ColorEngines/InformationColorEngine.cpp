/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include "InformationColorEngine.h"
#include <GUI/Utils/ColorRange.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/CategoryAdapter.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::ColorEngines;

const QList<QVariant::Type> NUMERICAL_TYPES = { QVariant::Int, QVariant::UInt, QVariant::LongLong, QVariant::ULongLong, QVariant::Double, QVariant::Bool };

//-----------------------------------------------------------------------------
InformationColorEngine::InformationColorEngine()
: ColorEngine("PropertyColorEngine", tr("Color by a property value."))
, m_key("MorphologicalInformation", "Size")
, m_colorRange(new RangeHSV(0, 10000))
, m_extension{nullptr}
{
}

//-----------------------------------------------------------------------------
InformationColorEngine::~InformationColorEngine()
{
  delete m_colorRange;
}

//-----------------------------------------------------------------------------
void InformationColorEngine::setInformation(const SegmentationExtension::InformationKey &key, double min, double max)
{
  m_key = key;

  m_colorRange->setMinimumValue(min);
  m_colorRange->setMaximumValue(max);

  emit modified();
}

//-----------------------------------------------------------------------------
void InformationColorEngine::setInformation(const Core::SegmentationExtension::InformationKey& key, const QStringList categories)
{
  m_key = key;

  m_categories = categories;
  m_colorRange->setMinimumValue(0);
  m_colorRange->setMaximumValue(m_categories.size()-1);

  emit modified();
}


//-----------------------------------------------------------------------------
QColor InformationColorEngine::color(ConstSegmentationAdapterPtr segmentation)
{
  Q_ASSERT(segmentation);

  QColor color(40,40,40); // very dark gray

  if(m_extension && m_extension->validCategory(segmentation->category()->classificationName()) && m_extension->validData(segmentation->output()))
  {
    auto extensions = segmentation->readOnlyExtensions();

    if (extensions->isReady(m_key))
    {
      auto info = extensions->information(m_key);

      if (info.isValid())
      {
        if(NUMERICAL_TYPES.contains(info.type()))
        {
          color = m_colorRange->color(info.toDouble());
        }
        else
        {
          if(info.type() == QVariant::String || info.canConvert<QString>())
          {
            auto category = info.toString();
            if(m_categories.contains(category))
            {
              auto pos = m_categories.indexOf(category);

              color = m_colorRange->color(pos);
            }
          }
        }
      }
    }
  }

  return color;
}

//-----------------------------------------------------------------------------
LUTSPtr InformationColorEngine::lut(ConstSegmentationAdapterPtr segmentation)
{
  auto  segColor = color(segmentation);
  auto  segLUT   = LUTSPtr::New();

  segLUT->Allocate();
  segLUT->SetNumberOfTableValues(2);
  segLUT->Build();
  segLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  segLUT->SetTableValue(1, segColor.redF(), segColor.greenF(), segColor.blueF(), 1.0);
  segLUT->Modified();

  return segLUT;
}
