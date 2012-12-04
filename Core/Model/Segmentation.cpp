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

#include "Core/Model/Filter.h"
#include "Core/Model/Channel.h"
#include "Core/ColorEngines/IColorEngine.h"

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>

#include <QDebug>

using namespace std;

const ModelItem::ArgumentId Segmentation::NUMBER   = "Number";
const ModelItem::ArgumentId Segmentation::OUTPUT   = "Output";
const ModelItem::ArgumentId Segmentation::TAXONOMY = "Taxonomy";
const ModelItem::ArgumentId Segmentation::USERS    = "Users";

const QString Segmentation::COMPOSED_LINK          = "ComposedOf";

//-----------------------------------------------------------------------------
Segmentation::SArguments::SArguments(const ModelItem::Arguments args)
: Arguments(args)
{
  m_number = args[NUMBER].toInt();
  m_outputId = args[OUTPUT].toInt();
}

//-----------------------------------------------------------------------------
QString Segmentation::SArguments::serialize() const
{
  /*TODO 2012-10-05 QString user = EspinaCore::instance()->settings().userName();
  SArguments *args = const_cast<SArguments *>(this);
  args->addUser(user);
  */
  return ModelItem::Arguments::serialize();
}

//-----------------------------------------------------------------------------
Segmentation::Segmentation(Filter* filter, Filter::OutputId oId)
: m_filter(filter)
, m_taxonomy(NULL)
, m_isVisible(true)
, m_padfilter(NULL)
, m_march(NULL)
{
  m_isSelected = false;
  //   memset(m_bounds, 0, 6*sizeof(double));
  //   m_bounds[1] = -1;
  m_args.setNumber(0);
  m_args.setOutputId(oId);
  m_args[TAXONOMY] = "Unknown";
  connect(filter, SIGNAL(modified(ModelItem *)),
          this, SLOT(notifyModification()));
}

//------------------------------------------------------------------------
void Segmentation::changeFilter(Filter* filter, Filter::OutputId oId)
{
  disconnect(m_filter, SIGNAL(modified(ModelItem *)),
             this, SLOT(notifyModification()));
//   m_filter->releaseDataFlagOn();
//   filter->releaseDataFlagOff();
  Filter::Output output = filter->output(oId);
  filter->update();
  m_filter = filter;
  m_args.setOutputId(oId);
  connect(filter, SIGNAL(modified(ModelItem *)),
          this, SLOT(notifyModification()));

  // update modified mesh extent to get a correct representation
  if (NULL != m_padfilter)
  {
    int extent[6];
    vtkImageData *image = vtkImageData::SafeDownCast(output.volume->toVTK()->GetProducer()->GetOutputDataObject(0));
    image->GetExtent(extent);
    m_padfilter->SetInputConnection(output.volume->toVTK());
    m_padfilter->SetOutputWholeExtent(extent[0]-1, extent[1]+1, extent[2]-1, extent[3]+1, extent[4]-1, extent[5]+1);
    m_padfilter->Update();
  }
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
QVariant Segmentation::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return QString("Segmentation %1").arg(m_args.number());
    case Qt::DecorationRole:
    {
      if (m_taxonomy)
        return m_taxonomy->color();
      else
        return QColor(Qt::red);
    }
    case Qt::ToolTipRole:
    {
      QString boundsInfo;
      QString filterInfo;
      if (m_filter && outputId() != Filter::Output::INVALID_OUTPUT_ID)
      {
        double bounds[6];
        volume()->bounds(bounds);
        boundsInfo = tr("<b>Sections:</b><br>");
        boundsInfo = boundsInfo.append("  X: %1 nm-%2 nm <br>").arg(bounds[0]).arg(bounds[1]);
        boundsInfo = boundsInfo.append("  Y: %1 nm-%2 nm <br>").arg(bounds[2]).arg(bounds[3]);
        boundsInfo = boundsInfo.append("  Z: %1 nm-%2 nm <br>").arg(bounds[4]).arg(bounds[5]);

        filterInfo = tr("<b>Filter:</b> %1<br>").arg(filter()->data().toString());
      }

      QString taxonomyInfo;
      if (m_taxonomy)
      {
        taxonomyInfo = tr("<b>Taxonomy:</b> %1<br>").arg(m_taxonomy->qualifiedName());
      }

      QString tooltip;
      tooltip = tooltip.append("<b>Name:</b> %1<br>").arg(data().toString());
      tooltip = tooltip.append(taxonomyInfo);
      tooltip = tooltip.append(filterInfo);
      tooltip = tooltip.append("<b>Users:</b> %1<br>").arg(m_args[USERS]);
      tooltip = tooltip.append(boundsInfo);

      if (!m_conditions.isEmpty())
      {
        tooltip = tooltip.append("<b>Conditions:</b><br>");
        foreach(ConditionInfo condition, m_conditions)
          tooltip = tooltip.append("<img src='%1' width=16 height=16>: %2<br>").arg(condition.first).arg(condition.second);
      }

      return tooltip;
    }
    case Qt::CheckStateRole:
      return visible() ? Qt::Checked : Qt::Unchecked;
      // //     case Qt::FontRole:
      // //     {
      // //       QFont myFont;
      // //          if (this->availableInformations().contains("Discarted"))
      // //          {
      // // 	          myFont.setStrikeOut(!visible());
      // //          }
      // //       return myFont;
      // //     }
    case Qt::UserRole + 1:
      return number();
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
  // Prevent overriding segmentation id assigned from model
  if (ModelItem::Arguments() != args)
    m_args = SArguments(args);
}

//------------------------------------------------------------------------
void Segmentation::initializeExtensions(ModelItem::Arguments args)
{
//   qDebug() << "Initializing" << data().toString() << "extensions:";
  foreach(ModelItemExtension *ext, m_insertionOrderedExtensions)
  {
    Q_ASSERT(ext);
    ModelItem::Arguments extArgs(args.value(ext->id(), QString()));
//     qDebug() << ext->id();
//     if (!args.isEmpty()) qDebug() << "*" << extArgs;
    ext->initialize(extArgs);
  }
}

//------------------------------------------------------------------------
void Segmentation::updateCacheFlag()
{
  m_filter->output(m_args.outputId()).isCached = true;
}

//------------------------------------------------------------------------
Channel* Segmentation::channel()
{
  ChannelList channels;

  ModelItem::Vector relatedChannels = relatedItems(ModelItem::IN, Channel::LINK);
  foreach(ModelItem *item, relatedChannels)
  {
    Q_ASSERT(CHANNEL == item->type());
    channels << dynamic_cast<Channel *>(item);
  }
  Q_ASSERT(channels.size() == 1);

  return channels.first();
}

//------------------------------------------------------------------------
SegmentationVolume::Pointer Segmentation::volume()
{
  EspinaVolume::Pointer ev = m_filter->volume(m_args.outputId());

  // On invalid cast:
  // The static_pointer_cast will "just do it". This will result in an invalid pointer and will likely cause a crash. The reference count on base will be incremented.
  // The shared_polymorphic_downcast will come the same as a static cast, but will trigger an assertion in the process. The reference count on base will be incremented.
  // The dynamic_pointer_cast will simply come out NULL. The reference count on base will be unchanged.
  return boost::dynamic_pointer_cast<SegmentationVolume>(ev);
}

//------------------------------------------------------------------------
const SegmentationVolume::Pointer Segmentation::volume() const
{
  EspinaVolume::Pointer ev = m_filter->volume(m_args.outputId());

  // On invalid cast:
  // The static_pointer_cast will "just do it". This will result in an invalid pointer and will likely cause a crash. The reference count on base will be incremented.
  // The shared_polymorphic_downcast will come the same as a static cast, but will trigger an assertion in the process. The reference count on base will be incremented.
  // The dynamic_pointer_cast will simply come out NULL. The reference count on base will be unchanged.
  return boost::dynamic_pointer_cast<SegmentationVolume>(ev);

}

//------------------------------------------------------------------------
void Segmentation::notifyModification(bool force)
{
  m_filter->volume(m_args.outputId())->update();

  ModelItem::notifyModification(force);
}


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
void Segmentation::setTaxonomy(TaxonomyElement* tax)
{
	m_taxonomy = tax;
	m_args[TAXONOMY] = Argument(tax->qualifiedName());
}

// void Segmentation::bounds(double val[3])
// //------------------------------------------------------------------------
// {
//   if (m_bounds[1] < m_bounds[0])
//   {
//     output()->GetProducer()->Update();
//     vtkImageData *image = vtkImageData::SafeDownCast(output());
//     image->GetBounds(m_bounds);
//   }
//   memcpy(val,m_bounds,6*sizeof(double));
// }

//------------------------------------------------------------------------
void Segmentation::setVisible(bool visible)
{
  m_isVisible = visible;
}

//------------------------------------------------------------------------
SegmentationList Segmentation::components()
{
  SegmentationList res;

  ModelItem::Vector subComponents = relatedItems(OUT, COMPOSED_LINK);

  foreach(ModelItem *item, subComponents)
  {
    Q_ASSERT(SEGMENTATION == item->type());
    res << dynamic_cast<Segmentation *>(item);
  }

  return res;
}

//------------------------------------------------------------------------
void Segmentation::addExtension(SegmentationExtension* ext)
{
  ModelItem::addExtension(ext);
  ext->setSegmentation(this);
}

//------------------------------------------------------------------------
QStringList Segmentation::availableInformations() const
{
  QStringList informations;
  informations << "Name" << "Taxonomy";
  informations << ModelItem::availableInformations();

  return informations;
}

//------------------------------------------------------------------------
QVariant Segmentation::information(QString info)
{
  if (info == "Name")
    return data(Qt::DisplayRole);
  if (info == "Taxonomy")
    return m_taxonomy->qualifiedName();

  Q_ASSERT(m_informations.contains(info));
  return m_informations[info]->information(info);
}

//------------------------------------------------------------------------
ModelItemExtension* Segmentation::extension(QString name)
{
  return ModelItem::extension(name);
}

vtkAlgorithmOutput* Segmentation::mesh()
{
  if (NULL == m_padfilter)
  {
    vtkAlgorithmOutput *vtkVolume = volume()->toVTK();
    // segmentation image need to be padded to avoid segmentation voxels from touching the edges of the
    // image (and create morphologically correct actors)
    int extent[6];
    vtkImageData *image = vtkImageData::SafeDownCast(vtkVolume->GetProducer()->GetOutputDataObject(0));
    image->GetExtent(extent);

    m_padfilter = vtkSmartPointer<vtkImageConstantPad>::New();
    m_padfilter->SetInputConnection(vtkVolume);
    m_padfilter->SetOutputWholeExtent(extent[0]-1, extent[1]+1, extent[2]-1, extent[3]+1, extent[4]-1, extent[5]+1);
    m_padfilter->SetConstant(0);

    m_march = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
    m_march->ReleaseDataFlagOn();
    m_march->SetNumberOfContours(1);
    m_march->GenerateValues(1, 255, 255);
    m_march->ComputeScalarsOff();
    m_march->ComputeNormalsOff();
    m_march->ComputeGradientsOff();
    m_march->SetInputConnection(m_padfilter->GetOutputPort());
  }

  return m_march->GetOutput()->GetProducerPort();
}

