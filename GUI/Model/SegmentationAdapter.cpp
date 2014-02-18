/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#include "SegmentationAdapter.h"

// // EspINA
#include <QPixmap>
#include <Core/Analysis/Segmentation.h>
#include <GUI/Model/CategoryAdapter.h>


using namespace EspINA;

//------------------------------------------------------------------------
SegmentationAdapter::SegmentationAdapter(FilterAdapterSPtr filter, SegmentationSPtr segmentation)
: ViewItemAdapter(filter, segmentation)
, m_segmentation{segmentation}
{
}

//------------------------------------------------------------------------
SegmentationAdapter::~SegmentationAdapter()
{

}

//------------------------------------------------------------------------
InputSPtr SegmentationAdapter::asInput() const
{
  return m_segmentation->asInput();
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
bool SegmentationAdapter::hasExtension(const SegmentationExtension::Type& type) const
{
  return m_segmentation->hasExtension(type);
}

//------------------------------------------------------------------------
SegmentationExtensionSPtr SegmentationAdapter::extension(const SegmentationExtension::Type& type) const
{
  return m_segmentation->extension(type);
}

//------------------------------------------------------------------------
SegmentationExtensionSList SegmentationAdapter::extensions() const
{
  return m_segmentation->extensions();
}


//------------------------------------------------------------------------
void SegmentationAdapter::addExtension(SegmentationExtensionSPtr extension)
{
  m_segmentation->addExtension(extension);
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
      QString value = m_segmentation->name();
      if (value.isEmpty())
      {
        value = QString("%1 %2").arg(m_category?m_category->name():"Unkown Category")
                                .arg(m_segmentation->number());
      }
      return value;
    }
    case Qt::DecorationRole:
    {
      const unsigned char WIDTH = 3;
      QPixmap segIcon(WIDTH, 16);
      segIcon.fill(m_category->color());

//       if (!information(SegmentationNotes::NOTE).toString().isEmpty())
//       {
//         QPixmap noteIcon(":/espina/note.png");
//         noteIcon = noteIcon.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//         
//         const unsigned char SP = 5;
//         QPixmap tmpIcon(WIDTH + SP + noteIcon.width(),16);
//         tmpIcon.fill(Qt::white);
//         QPainter painter(&tmpIcon);
//         painter.drawPixmap(0,0, segIcon);
//         painter.drawPixmap(WIDTH + SP,0, noteIcon);
//         
//         segIcon = tmpIcon;
//       }
      
      return segIcon;
    }
    case Qt::ToolTipRole:
    {
      const QString WS  = "&nbsp;"; // White space
      const QString TAB = WS+WS+WS;
      QString boundsInfo;
      QString filterInfo;
      if (m_filter && output()->isValid()) //FIXME: Utilizar el region del output
      {
        Bounds bounds = output()->bounds();
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

      for(auto extension : m_segmentation->extensions())
      {
//         if (extension->isEnabled())
//         {
        QString extToolTip = extension->toolTipText();
        if (!extToolTip.isEmpty())
        {
          if (addBreakLine && !extToolTip.contains("</table>")) tooltip = tooltip.append("<br>");

          tooltip = tooltip.append(extToolTip);

          addBreakLine = true;
//           }
        }
      }
      return tooltip;
    }
    case Qt::CheckStateRole:
      return isVisible() ? Qt::Checked : Qt::Unchecked;
    case TypeRole:
      return typeId(Type::SEGMENTATION);
//     case SegmentationNumberRole:
//       return number();
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
QVariant SegmentationAdapter::information(const SegmentationExtension::InfoTag& tag) const
{
  return m_segmentation->information(tag);
}

//------------------------------------------------------------------------
SegmentationExtension::InfoTagList SegmentationAdapter::informationTags() const
{
  return m_segmentation->informationTags();
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
    case SelectionRole:
      setSelected(value.toBool());
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
bool EspINA::operator==(SegmentationAdapterSPtr lhs, SegmentationSPtr rhs)
{
  return lhs->m_segmentation == rhs;
}

//------------------------------------------------------------------------
bool EspINA::operator==(SegmentationSPtr lhs, SegmentationAdapterSPtr rhs)
{
  return lhs == rhs->m_segmentation;
}


//------------------------------------------------------------------------
bool EspINA::operator!=(SegmentationAdapterSPtr lhs, SegmentationSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
bool EspINA::operator!=(SegmentationSPtr lhs, SegmentationAdapterSPtr rhs)
{
  return !operator==(lhs, rhs);
}

//------------------------------------------------------------------------
SegmentationAdapterPtr EspINA::segmentationPtr(ViewItemAdapterPtr item)
{
  return dynamic_cast<SegmentationAdapterPtr>(item);
}