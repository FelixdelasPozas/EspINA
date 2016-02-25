/*

    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>

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
#include <Extensions/Issues/SegmentationIssues.h>

// Qt
#include <QPixmap>
#include <QPainter>

using namespace ESPINA;
using namespace ESPINA::Extensions;

//------------------------------------------------------------------------
SegmentationAdapter::SegmentationAdapter(SegmentationSPtr segmentation)
: ViewItemAdapter(segmentation)
, m_segmentation{segmentation}
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
      QString value = m_segmentation->alias();

      if(value.isEmpty())
      {
        value = m_segmentation->name();
      }

      if (value.isEmpty())
      {
        value = QString("%1 %2").arg(m_category?m_category->name():"Unknown Category")
                                .arg(m_segmentation->number());
      }

      return value;
    }
    case Qt::DecorationRole:
    {
      QPixmap icon(3, 16);

      // Category icon
      icon.fill(m_category->color());

      // We should let the extensions decorate
      if (hasInformation(SegmentationIssues::ISSUES))
      {
        if (information(SegmentationIssues::CRITICAL).toInt() > 0)
        {
          icon = appendImage(icon, SegmentationIssues::severityIcon(Issue::Severity::CRITICAL, true), true);
        }
        else if (information(SegmentationIssues::WARNING).toInt() > 0)
        {
          icon = appendImage(icon, SegmentationIssues::severityIcon(Issue::Severity::WARNING, true), true);
        }
      }

      if (!information(SegmentationNotes::NOTES).toString().isEmpty())
      {
        icon = appendImage(icon, ":/espina/note.svg");
      }

      return icon;
    }
    case Qt::ToolTipRole:
    {
      const QString WS  = "&nbsp;"; // White space
      const QString TAB = WS+WS+WS;
      QString boundsInfo;
      QString filterInfo;
      //if (m_filter && output()->isValid())
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
        boundsInfo = boundsInfo.append(TAB+"X: [%1 nm, %2 nm)<br>").arg(bounds[0]).arg(bounds[1]);
        boundsInfo = boundsInfo.append(TAB+"Y: [%1 nm, %2 nm)<br>").arg(bounds[2]).arg(bounds[3]);
        boundsInfo = boundsInfo.append(TAB+"Z: [%1 nm, %2 nm)").arg(bounds[4]).arg(bounds[5]);

//         //filterInfo = tr("<b>Filter:</b><br> %1<br>").arg(TAB+filter()->data().toString());
//         filterInfo = m_filter->data(Qt::ToolTipRole).toString();
      }

      QString categoryInfo;
      if (m_category)
      {
        categoryInfo = tr("<b>Category:</b> %1<br>").arg(m_category->classificationName());
      }

      QString tooltip;
      tooltip = tooltip.append("<center><b>%1</b></center>").arg(data().toString());
      tooltip = tooltip.append(categoryInfo);
      //tooltip = tooltip.append("<b>Users:</b> %1<br>").arg(m_args[USERS]);
      tooltip = tooltip.append(boundsInfo);
      bool addBreakLine = false;

      if (!filterInfo.isEmpty())
      {
        tooltip      = tooltip.append(filterInfo);
        addBreakLine = true;
      }

      for(auto extension : m_segmentation->readOnlyExtensions())
      {
        QString extToolTip = extension->toolTipText();
        if (!extToolTip.isEmpty())
        {
          if (addBreakLine && !extToolTip.contains("</table>")) tooltip = tooltip.append("<br>");

          tooltip = tooltip.append(extToolTip);

          addBreakLine = true;
        }
      }

      return tooltip;
    }
    case Qt::CheckStateRole:
      return isVisible() ? Qt::Checked : Qt::Unchecked;
    case TypeRole:
      return typeId(Type::SEGMENTATION);
    case NumberRole:
      return number();
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
Extension< Segmentation >::InformationKeyList SegmentationAdapter::availableInformation() const
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
    case TypeRole: // Before it had the same value but it was SelectionRole
      Q_ASSERT(false);
      //setSelected(value.toBool());
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
QPixmap SegmentationAdapter::appendImage(const QPixmap& original, const QString& image, bool slim) const
{
  QPixmap pixmap(image);

  pixmap = pixmap.scaled(slim?8:16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  const unsigned char SP = 5;

  QPixmap tmpPixmap(original.width() + SP + pixmap.width(),16);
  tmpPixmap.fill(Qt::transparent);

  QPainter painter(&tmpPixmap);
  painter.drawPixmap(0,0, original);
  painter.drawPixmap(original.width() + SP,0, pixmap);

  return tmpPixmap;
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
