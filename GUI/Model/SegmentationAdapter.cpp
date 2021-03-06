/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// // ESPINA
#include "SegmentationAdapter.h"
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <GUI/Model/CategoryAdapter.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <Extensions/Issues/Issues.h>
#include <Extensions/Issues/ItemIssues.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Utils/MiscUtils.h>

// Qt
#include <QPixmap>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI::Model::Utils;

//------------------------------------------------------------------------
SegmentationAdapter::SegmentationAdapter(SegmentationSPtr segmentation)
: ViewItemAdapter(segmentation)
, m_segmentation {segmentation}
, m_category     {nullptr}
, m_colorEngine  {nullptr}
{
  connect(m_segmentation.get(), SIGNAL(outputModified()),
          this,                 SIGNAL(outputModified()));
}

//------------------------------------------------------------------------
SegmentationAdapter::~SegmentationAdapter()
{
  disconnect(m_segmentation.get(), SIGNAL(outputModified()),
             this,                 SIGNAL(outputModified()));
}

//------------------------------------------------------------------------
InputSPtr SegmentationAdapter::asInput() const
{
  return m_segmentation->asInput();
}

//------------------------------------------------------------------------
void SegmentationAdapter::changeOutputImplementation(InputSPtr input)
{
  m_segmentation->changeOutput(input);
}

//------------------------------------------------------------------------
void SegmentationAdapter::setNumber(unsigned int number)
{
  m_segmentation->setNumber(number);
}

//------------------------------------------------------------------------
unsigned int SegmentationAdapter::number() const
{
  return m_segmentation->number();
}

//------------------------------------------------------------------------
Bounds SegmentationAdapter::bounds() const
{
  return m_segmentation->bounds();
}

//------------------------------------------------------------------------
SegmentationAdapter::ReadLockExtensions SegmentationAdapter::readOnlyExtensions() const
{
  return m_segmentation->readOnlyExtensions();
}

//------------------------------------------------------------------------
SegmentationAdapter::WriteLockExtensions SegmentationAdapter::extensions()
{
  return m_segmentation->extensions();
}

//------------------------------------------------------------------------
CategoryAdapterSPtr SegmentationAdapter::category() const
{
  return m_category;
}

//------------------------------------------------------------------------
QVariant SegmentationAdapter::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    {
      auto value = m_segmentation->alias().isEmpty() ? m_segmentation->name() : m_segmentation->alias();

      if (value.isEmpty())
      {
        value = categoricalName(const_cast<SegmentationAdapterPtr>(this));

        m_segmentation->setName(value);
        m_segmentation->setAlias(value);
      }

      return value;
    }
    case Qt::DecorationRole:
    {
      QPixmap icon(3, 16);

      // Category icon
      if(m_category) icon.fill(m_category->color());
      else           icon.fill(Qt::red);

      // We should let the extensions decorate
      if (hasInformation(SegmentationIssues::ISSUES))
      {
        if (information(SegmentationIssues::CRITICAL).toInt() > 0)
        {
          icon = GUI::Utils::appendImage(icon, SegmentationIssues::severityIcon(Issue::Severity::CRITICAL, true), true);
        }
        else if (information(SegmentationIssues::WARNING).toInt() > 0)
        {
          icon = GUI::Utils::appendImage(icon, SegmentationIssues::severityIcon(Issue::Severity::WARNING, true), true);
        }
      }

      if (!information(SegmentationNotes::NOTES).toString().isEmpty())
      {
        icon = GUI::Utils::appendImage(icon, ":/espina/note.svg");
      }

      return icon;
    }
    case Qt::ToolTipRole:
    {
      const QString WS  = "&nbsp;"; // White space
      const QString TAB = WS+WS+WS;
      QString boundsInfo;

      if (output()->isValid()) // It shouldn't exist a segmentation without filter as it was checked before, but maybe there is some weird condition in which we should check it
      {
        Bounds bounds;
        if(hasVolumetricData(output()))
        {
          // voxel bounds are the preferred ones to show.
          bounds = readLockVolume(output())->bounds();
        }
        else
        {
          bounds = output()->bounds();
        }
        boundsInfo = tr("<b>Bounds:</b><br>");
        boundsInfo = boundsInfo.append(TAB+"X: [%1 nm, %2 nm]<br>").arg(bounds[0]).arg(bounds[1]);
        boundsInfo = boundsInfo.append(TAB+"Y: [%1 nm, %2 nm]<br>").arg(bounds[2]).arg(bounds[3]);
        boundsInfo = boundsInfo.append(TAB+"Z: [%1 nm, %2 nm]<br>").arg(bounds[4]).arg(bounds[5]);
      }

      QString categoryInfo;
      if (m_category)
      {
        categoryInfo = tr("<b>Category:</b> %1<br>").arg(m_category->classificationName());
      }

      QString tooltip;
      tooltip = tooltip.append("<center><b>%1</b></center>").arg(data().toString());
      tooltip = tooltip.append(categoryInfo);
      tooltip = tooltip.append(boundsInfo);

      auto cleanTextBR = [] (QString &text)
      {
        while(text.endsWith("<br>"))
        {
          auto index = text.lastIndexOf("<br>");
          if(index != -1)
          {
            text = text.remove(index, 4);
          }
          else
          {
            break;
          }
        }
      };

      QString extTooltip;
      for(auto extension : m_segmentation->readOnlyExtensions())
      {
        auto extensionTooltip = extension->toolTipText();
        if (!extensionTooltip.isEmpty())
        {
          cleanTextBR(extensionTooltip);
          extTooltip = extTooltip.append(extensionTooltip).append("<br>");
        }
      }

      if(!extTooltip.isEmpty())
      {
        tooltip = tooltip.append(tr("<b>Extensions:</b><br>%1").arg(extTooltip));
      }

      cleanTextBR(tooltip);

      return tooltip;
    }
    case Qt::CheckStateRole:
      return isVisible() ? Qt::Checked : Qt::Unchecked;
    case TypeRole:
      return typeId(Type::SEGMENTATION);
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
QVariant SegmentationAdapter::information(const SegmentationExtension::InformationKey &key) const
{
  return m_segmentation->information(key);
}

//------------------------------------------------------------------------
bool SegmentationAdapter::isReady(const SegmentationExtension::InformationKey &key) const
{
  return m_segmentation->readOnlyExtensions()->isReady(key);
}

//------------------------------------------------------------------------
const Extension<Segmentation>::InformationKeyList SegmentationAdapter::availableInformation() const
{
  return m_segmentation->readOnlyExtensions()->availableInformation();
}

//------------------------------------------------------------------------
void SegmentationAdapter::modifiedByUser(const QString& user)
{
  m_segmentation->modifiedByUser(user);
}

//------------------------------------------------------------------------
void SegmentationAdapter::setCategory(CategoryAdapterSPtr category)
{
  m_segmentation->setCategory(category->m_category);
  m_category = category;
}

//------------------------------------------------------------------------
bool SegmentationAdapter::setData(const QVariant& value, int role)
{
  switch (role)
  {
    case Qt::EditRole:
      m_segmentation->setAlias(value.toString());
      return true;
    case Qt::CheckStateRole:
      setVisible(value.toBool());
      return true;
    default:
      return false;
  }
}

//------------------------------------------------------------------------
QStringList SegmentationAdapter::users() const
{
  return m_segmentation->users();
}

//------------------------------------------------------------------------
GUI::ColorEngines::ColorEngineSPtr SegmentationAdapter::colorEngine() const
{
  return m_colorEngine;
}

//------------------------------------------------------------------------
void SegmentationAdapter::setColorEngine(GUI::ColorEngines::ColorEngineSPtr engine)
{
  if(engine && engine != m_colorEngine)
  {
    m_colorEngine = engine;
  }
}

//------------------------------------------------------------------------
void ESPINA::SegmentationAdapter::clearColorEngine()
{
  m_colorEngine = nullptr;
}

//------------------------------------------------------------------------
bool ESPINA::operator==(SegmentationAdapterSPtr lhs, SegmentationSPtr rhs)
{
  return lhs->m_segmentation == rhs;
}

//------------------------------------------------------------------------
bool ESPINA::operator==(SegmentationSPtr lhs, SegmentationAdapterSPtr rhs)
{
  return lhs == rhs->m_segmentation;
}

//------------------------------------------------------------------------
bool ESPINA::operator!=(SegmentationAdapterSPtr lhs, SegmentationSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
bool ESPINA::operator!=(SegmentationSPtr lhs, SegmentationAdapterSPtr rhs)
{
  return !operator==(lhs, rhs);
}
