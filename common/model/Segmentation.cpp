/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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
#include "Segmentation.h"

#include "Filter.h"

#include <QDebug>
#include <vtkPVDataInformation.h>
#include <pqOutputPort.h>
#include <pqPipelineSource.h>

using namespace std;

const ModelItem::ArgumentId Segmentation::FILTER    = ArgumentId("Filter",   ArgumentId::KEY);
const ModelItem::ArgumentId Segmentation::NUMBER    = ArgumentId("Number",   ArgumentId::KEY);
const ModelItem::ArgumentId Segmentation::OUTPUT    = ArgumentId("Output",   ArgumentId::KEY);
const ModelItem::ArgumentId Segmentation::TAXONOMY  = ArgumentId("Taxonomy", ArgumentId::VARIABLE);

//-----------------------------------------------------------------------------
Segmentation::Segmentation(Filter* filter, int output, pqData data)
: m_filter (filter)
, m_data   (data)
, m_taxonomy    (NULL)
, m_isSelected(false)
, m_isVisible(true)
{
  memset(m_bounds, 0, 6*sizeof(double));
  m_bounds[1] = -1;
  m_args.setNumber(0);
  m_args[FILTER] = m_filter->id();
  m_args.setOutput(output);
  m_args[TAXONOMY] = "Unknown";
}

//------------------------------------------------------------------------
Segmentation::~Segmentation()
{
//   int size = m_insertionOrderedExtensions.size()-1;
//   for (int i = size; i >= 0; i--)
//     delete m_insertionOrderedExtensions[i];
//   
//   foreach(ISegmentationExtension *ext, m_pendingExtensions)
//     delete ext;
//   
//   m_extensions.clear();
//   m_pendingExtensions.clear();
//   m_insertionOrderedExtensions.clear();
//   m_representations.clear();
//   m_informations.clear();
}

//------------------------------------------------------------------------
pqOutputPort* Segmentation::outputPort()
{
  return m_data.outputPort();
}

//------------------------------------------------------------------------
QString Segmentation::id() const
{
  return m_filter->id() + "_" + m_args[OUTPUT];
}

//------------------------------------------------------------------------
QVariant Segmentation::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
//     //case Qt::EditRole:
      return QString("Segmentation %1").arg(m_args.number());
//     case Qt::DecorationRole:
//     {
//       QPixmap segIcon(3,16);
//       segIcon.fill(m_taxonomy->color());
//       return segIcon;
//     }
    case Qt::ToolTipRole:
      return QString(
	"<b>Name:</b> %1<br>"
// 	"<b>Taxonomy:</b> %2<br>"
// 	"<b>Sample:</b> %3<br>"
// 	"<b>Created by:</b><br>"
// 	"%4"
      )
      .arg(data(Qt::DisplayRole).toString())
//       .arg(m_taxonomy->qualifiedName())
//       .arg(origin()->label())
//       .arg(m_parent->getFilterArguments())
      ;
    case Qt::CheckStateRole:
      return visible()?Qt::Checked:Qt::Unchecked;
// //     case Qt::FontRole:
// //     {
// //       QFont myFont;
// // //       if (this->availableInformations().contains("Discarted"))
// // //       {
// // 	myFont.setStrikeOut(!visible());
// // //       }
// //       return myFont;
// //     }
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
QString Segmentation::serialize() const
{
  return m_args.serialize();
}

//------------------------------------------------------------------------
void Segmentation::initialize(ModelItem::Arguments args)
{
  foreach(ModelItemExtension *ext, m_extensions)
  {
    SegmentationExtension *segExt = dynamic_cast<SegmentationExtension *>(ext);
    Q_ASSERT(segExt);
    segExt->initialize(this);
  }
}

// //------------------------------------------------------------------------
// void Segmentation::color(double* rgba)
// {
//   QColor color = m_taxonomy->color();
//   rgba[0] = color.red()/255.0;
//   rgba[1] = color.green()/255.0;
//   rgba[2] = color.blue()/255.0;
//   rgba[3] = 1;
// }

//------------------------------------------------------------------------
bool Segmentation::setData(const QVariant& value, int role)
{
  switch (role)
  {
    case Qt::EditRole:
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
void Segmentation::setTaxonomy(TaxonomyNode* tax)
{
  m_taxonomy = tax;
  m_args[TAXONOMY] = Argument(tax->qualifiedName());
}

void Segmentation::bounds(double val[3])
//------------------------------------------------------------------------
{
  if (m_bounds[1] < m_bounds[0])
  {
    m_data.pipelineSource()->updatePipeline();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    info->GetBounds(m_bounds);
  }
  memcpy(val,m_bounds,6*sizeof(double));
}
//------------------------------------------------------------------------
void Segmentation::setSelected(bool selected)
{
  m_isSelected = selected;
}

//------------------------------------------------------------------------
void Segmentation::setVisible(bool visible)
{
  m_isVisible = visible;
}


//------------------------------------------------------------------------
void Segmentation::addExtension(SegmentationExtension * ext)
{
  ModelItem::addExtension(ext);
}

// //------------------------------------------------------------------------
// ISegmentationExtension *Segmentation::extension(ExtensionId extId)
// {
//   assert(m_extensions.contains(extId));
//   return m_extensions[extId];
// }
// 
// //------------------------------------------------------------------------
// QStringList Segmentation::availableRepresentations() const
// {
//   QStringList represnetations;
//   foreach (ISegmentationExtension *ext, m_insertionOrderedExtensions)
//     represnetations << ext->availableRepresentations();
//   
//   return represnetations;
// }
// 
// //------------------------------------------------------------------------
// ISegmentationRepresentation* Segmentation::representation(QString rep)
// {
//   return m_representations[rep]->representation(rep);
// }

//------------------------------------------------------------------------
QStringList Segmentation::availableInformations() const
{
  QStringList informations;
  informations << "Name" << "Taxonomy";
  informations << ModelItem::availableInformations();

  return informations;
}

//------------------------------------------------------------------------
QVariant Segmentation::information(QString info) const
{
  if (info == "Name")
    return data(Qt::DisplayRole);
  if (info == "Taxonomy")
    return m_taxonomy->qualifiedName();

  Q_ASSERT(m_informations.contains(info));
  return m_informations[info]->information(info);
}

// 
// //------------------------------------------------------------------------
// void Segmentation::notifyInternalUpdate()
// {
// //   std::cout << "Notifying update" << std::endl;
// //   this->origin()->representation(LabelMapExtension::SampleRepresentation::ID)->requestUpdate(true);
//   emit updated(this);
// }
