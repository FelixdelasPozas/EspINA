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

#include "Core/Model/EspinaFactory.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Filter.h"
#include "Core/Model/Channel.h"
#include "Core/Model/Taxonomy.h"
#include "EspinaModel.h"
#include "Core/ColorEngines/IColorEngine.h"
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Extensions/Tags/TagExtension.h>
#include <Core/Extensions/Notes/SegmentationNotes.h>
#include <Core/Relations.h>

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QPainter>

using namespace std;
using namespace EspINA;

const ModelItem::ArgumentId Segmentation::NUMBER    = "Number";
const ModelItem::ArgumentId Segmentation::OUTPUT    = "Output";
const ModelItem::ArgumentId Segmentation::TAXONOMY  = "Taxonomy";
const ModelItem::ArgumentId Segmentation::DEPENDENT = "Dependent";
const ModelItem::ArgumentId Segmentation::USERS     = "Users";


const int SegmentationNumberRole = TypeRole+1;

//-----------------------------------------------------------------------------
Segmentation::SArguments::SArguments(const Arguments &args)
: Arguments(args)
{
  m_number = args[NUMBER].toInt();
  m_outputId = args[OUTPUT].toInt();
  m_isInputSegmentationDependent = args.value(DEPENDENT, "0").toInt();
}

//-----------------------------------------------------------------------------
QString Segmentation::SArguments::serialize() const
{
  return ModelItem::Arguments::serialize();
}

//-----------------------------------------------------------------------------
Segmentation::Segmentation(FilterSPtr         filter,
                           const Filter::OutputId &oId)
: PickableItem()
, m_filter(filter)
, m_taxonomy(NULL)
, m_isVisible(true)
, m_isInputSegmentationDependent(false)
{
  m_args.setNumber(0);
  m_args.setOutputId(oId);
  m_args[TAXONOMY] = "Unknown";
  connect(volume().get(), SIGNAL(modified()),
          this, SLOT(onVolumeModified()));
}

//------------------------------------------------------------------------
void Segmentation::changeFilter(FilterSPtr filter, const Filter::OutputId &oId)
{
  disconnect(volume().get(), SIGNAL(modified()),
             this, SLOT(onVolumeModified()));
//   m_filter->releaseDataFlagOn();
//   filter->releaseDataFlagOff();
  filter->update(oId);
  m_filter = filter;
  m_args.setOutputId(oId);
  connect(volume().get(), SIGNAL(modified()),
          this, SLOT(onVolumeModified()));
}

//------------------------------------------------------------------------
Segmentation::~Segmentation()
{
  //qDebug() << data().toString() << ": Destructor";
  foreach(InformationExtension extension, m_informationExtensions)
    delete extension;
}

//------------------------------------------------------------------------
QVariant Segmentation::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return QString("%1 %2").arg(m_taxonomy->name())
                             .arg(m_args.number());
    case Qt::DecorationRole:
    {
      const unsigned char WIDTH = 3;
      QPixmap segIcon(WIDTH, 16);
      segIcon.fill(taxonomy()->color());

      if (!information(SegmentationNotes::NOTE).toString().isEmpty())
      {
        QPixmap noteIcon(":/espina/note.png");
        noteIcon = noteIcon.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        const unsigned char SP = 5;
        QPixmap tmpIcon(WIDTH + SP + noteIcon.width(),16);
        tmpIcon.fill(Qt::white);
        QPainter painter(&tmpIcon);
        painter.drawPixmap(0,0, segIcon);
        painter.drawPixmap(WIDTH + SP,0, noteIcon);

        segIcon = tmpIcon;
      }

      return segIcon;
    }
    case Qt::ToolTipRole:
    {
      const QString WS  = "&nbsp;"; // White space
      const QString TAB = WS+WS+WS;
      QString boundsInfo;
      QString filterInfo;
      if (m_filter && outputId() != Filter::Output::INVALID_OUTPUT_ID)
      {
        double bounds[6];
        volume()->bounds(bounds);
        boundsInfo = tr("<b>Sections:</b><br>");
        boundsInfo = boundsInfo.append(TAB+"X: %1 nm - %2 nm <br>").arg(bounds[0]).arg(bounds[1]);
        boundsInfo = boundsInfo.append(TAB+"Y: %1 nm - %2 nm <br>").arg(bounds[2]).arg(bounds[3]);
        boundsInfo = boundsInfo.append(TAB+"Z: %1 nm - %2 nm").arg(bounds[4]).arg(bounds[5]);

        //filterInfo = tr("<b>Filter:</b><br> %1<br>").arg(TAB+filter()->data().toString());
        filterInfo = m_filter->data(Qt::ToolTipRole).toString();
      }

      QString taxonomyInfo;
      if (m_taxonomy)
      {
        taxonomyInfo = tr("<b>Taxonomy:</b> %1<br>").arg(m_taxonomy->qualifiedName());
      }

      QString tooltip;
      tooltip = tooltip.append("<b>Name:</b> %1<br>").arg(data().toString());
      tooltip = tooltip.append(taxonomyInfo);
      //tooltip = tooltip.append("<b>Users:</b> %1<br>").arg(m_args[USERS]);
      tooltip = tooltip.append(boundsInfo);
      bool addBreakLine = false;

      if (!filterInfo.isEmpty())
      {
        tooltip      = tooltip.append(filterInfo);
        addBreakLine = true;
      }

      // FIXME: Hack to ensure notes and tags extension are always loaded
      informationExtension(SegmentationTagsID);
      informationExtension(SegmentationNotesID);
      foreach (InformationExtension extension, m_informationExtensions)
      {
        QString extToolTip = extension->toolTipText();
        if (!extToolTip.isEmpty())
        {
          if (addBreakLine && !extToolTip.contains("</table>")) tooltip = tooltip.append("<br>");

          tooltip = tooltip.append(extToolTip);

          addBreakLine = true;
        }
      }

//       if (!m_conditions.isEmpty())
//       {
//         tooltip = tooltip.append("<b>Conditions:</b><br>");
//         foreach(ConditionInfo condition, m_conditions)
//           tooltip = tooltip.append("<img src='%1' width=16 height=16>: %2<br>").arg(condition.first).arg(condition.second);
//       }

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
    case TypeRole: // TODO 2013-01-03 remove or add to others (now is only availabe for seg and tax)
      return EspINA::SEGMENTATION;
    case SegmentationNumberRole:
      return number();
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
QString Segmentation::serialize() const
{
  // Ensure taxonomy name is updated
  m_args[TAXONOMY] = Argument(m_taxonomy->qualifiedName());
  return m_args.serialize();
}

//------------------------------------------------------------------------
void Segmentation::initialize(const Arguments &args)
{
  // Prevent overriding segmentation id assigned from model
  if (Arguments() != args)
    m_args = SArguments(args);
}

//------------------------------------------------------------------------
void Segmentation::initializeExtensions()
{
  //qDebug() << "Initializing" << data().toString() << "extensions:";
  foreach(Segmentation::InformationExtension ext, m_informationExtensions)
  {
    Q_ASSERT(ext);
    //qDebug() << "Initializing" << data().toString() << ext->id() << "extension";
    ext->initialize();
  }
}

//------------------------------------------------------------------------
void Segmentation::invalidateExtensions()
{
  foreach(InformationExtension infoExtension, m_informationExtensions)
  {
    infoExtension->invalidate();
  }
}

//------------------------------------------------------------------------
void Segmentation::updateCacheFlag()
{
  m_filter->output(m_args.outputId()).isCached = true;
}

//------------------------------------------------------------------------
SampleSPtr Segmentation::sample()
{
  SampleSPtr sample;

  ModelItemSList relatedChannels = relatedItems(EspINA::IN, Channel::LINK);
  // NOTE: Decide how to deal with segmentations created from various channels in a future
  if (relatedChannels.size() == 1)
  {
    ModelItemSPtr channel = relatedChannels.first();
    ModelItemSList relatedSamples = channel->relatedItems(EspINA::IN, Channel::STAIN_LINK);

    Q_ASSERT(relatedSamples.size() == 1);
    sample = samplePtr(relatedSamples.first());
  }

  return sample;
}

//------------------------------------------------------------------------
ChannelSPtr Segmentation::channel()
{
  ChannelSList channels;

  ModelItemSList relatedChannels = relatedItems(IN, Channel::LINK);

  ChannelSPtr channel;
  // NOTE: Decide how to deal with segmentations created from various channels in a future
  if (relatedChannels.size() == 1)
  {
    channel = channelPtr(relatedChannels.first());
  }

  return channel;
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

// //------------------------------------------------------------------------
// void Segmentation::notifyModification(bool force)
// {
//   m_filter->volume(m_args.outputId())->update();
// 
//   ModelItem::notifyModification(force);
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
void Segmentation::setTaxonomy(TaxonomyElementSPtr tax)
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
SegmentationSList Segmentation::components()
{
  SegmentationSList res;

  ModelItemSList subComponents = relatedItems(EspINA::OUT, Relations::COMPOSITION);

  foreach(ModelItemSPtr item, subComponents)
  {
    Q_ASSERT(SEGMENTATION == item->type());
    res << segmentationPtr(item);
  }

  return res;
}

//------------------------------------------------------------------------
SegmentationSList Segmentation::componentOf()
{
  SegmentationSList res;

  ModelItemSList subComponents = relatedItems(EspINA::IN, Relations::COMPOSITION);

  foreach(ModelItemSPtr item, subComponents)
  {
    Q_ASSERT(SEGMENTATION == item->type());
    res << segmentationPtr(item);
  }

  return res;
}

//------------------------------------------------------------------------
void Segmentation::addExtension(Segmentation::InformationExtension extension)
{
  if (extension->validTaxonomy(m_taxonomy->qualifiedName()))
  {
    if (m_informationExtensions.contains(extension->id()))
    {
      qWarning() << "Extension already registered";
      Q_ASSERT(false);
    }

    EspinaFactory *factory = m_model->factory();
    foreach(ModelItem::ExtId requiredExtensionId, extension->dependencies())
    {
      InformationExtension requiredExtension = informationExtension(requiredExtensionId);
      if (!requiredExtension)
      {
        InformationExtension prototype = factory->segmentationExtension(requiredExtensionId);
        if (!prototype)
        {
          qWarning() << "Failed to load extension's dependency" << requiredExtensionId;
          Q_ASSERT(false);
        }

        addExtension(prototype->clone());
      }
    }

    extension->setSegmentation(this);
    m_informationExtensions[extension->id()] = extension;

    foreach(InfoTag tag, extension->availableInformations())
    {
      Q_ASSERT(!m_informationTagProvider.contains(tag));
      m_informationTagProvider.insert(tag, extension);
    }
  }
}

//------------------------------------------------------------------------
bool Segmentation::hasInformationExtension(const ModelItem::ExtId &name) const
{
  return m_informationExtensions.contains(name);
}

//------------------------------------------------------------------------
Segmentation::InformationExtension Segmentation::informationExtension(const ModelItem::ExtId &name) const
{
  Segmentation::InformationExtension extension = m_informationExtensions.value(name, NULL);

  if (!extension)
  {
    InformationExtension prototype = m_model->factory()->segmentationExtension(name);
    if (prototype->validTaxonomy(m_taxonomy->qualifiedName()))
    {
      extension = prototype->clone();
      const_cast<Segmentation *>(this)->addExtension(extension);
    }
  }

  return extension;
}


//------------------------------------------------------------------------
Segmentation::InfoTagList Segmentation::availableInformations() const
{
  InfoTagList tags;

  tags << tr("Name") << tr("Taxonomy");

  foreach (InformationExtension ext, m_informationExtensions)
    tags << ext->availableInformations();

  return tags;
}

//------------------------------------------------------------------------
QVariant Segmentation::information(const Segmentation::InfoTag &tag) const
{
  if (tag == tr("Name"))
    return data(Qt::DisplayRole);
  if (tag == tr("Taxonomy"))
    return m_taxonomy->qualifiedName();

  if (!m_informationTagProvider.contains(tag))
  {
    InformationExtension prototype = m_model->factory()->informationProvider(tag);
    if (prototype->validTaxonomy(m_taxonomy->qualifiedName()))
    {
      const_cast<Segmentation *>(this)->addExtension(prototype->clone());
    }
  }
  Q_ASSERT(m_informationTagProvider.contains(tag));
  return m_informationTagProvider[tag]->information(tag);
}

// //------------------------------------------------------------------------
// void Segmentation::deleteExtension(Segmentation::InformationExtension extension)
// {
// 
// }

// //------------------------------------------------------------------------
// void Segmentation::checkExtensionCompability()
// {
// //   foreach(InformationExtension extension, m_informationExtensions)
// //   {
// //     if (!extension->isCompatible())
// //       deleteExtension(extension);
// //   }
// }


//------------------------------------------------------------------------
SegmentationPtr EspINA::segmentationPtr(ModelItemPtr item)
{
  Q_ASSERT(SEGMENTATION == item->type());
  SegmentationPtr ptr = dynamic_cast<SegmentationPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//------------------------------------------------------------------------
SegmentationPtr EspINA::segmentationPtr(PickableItemPtr item)
{
  Q_ASSERT(EspINA::SEGMENTATION == item->type());
  SegmentationPtr ptr = dynamic_cast<SegmentationPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//------------------------------------------------------------------------
SegmentationSPtr EspINA::segmentationPtr(ModelItemSPtr &item)
{
  Q_ASSERT(SEGMENTATION == item->type());
  SegmentationSPtr ptr = qSharedPointerDynamicCast<Segmentation>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;
}


//------------------------------------------------------------------------
SegmentationSPtr EspINA::segmentationPtr(PickableItemSPtr &item)
{
  Q_ASSERT(SEGMENTATION == item->type());
  SegmentationSPtr ptr = qSharedPointerDynamicCast<Segmentation>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;
}

