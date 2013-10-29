/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "Core/Analysis/Sample.h"

using namespace EspINA;

//------------------------------------------------------------------------
Sample::Sample(const QString& name)
: m_name{name}
{

}

//------------------------------------------------------------------------
Sample::~Sample()
{

}

//------------------------------------------------------------------------
void Sample::restoreState(const State& state)
{

}

//------------------------------------------------------------------------
void Sample::saveState(State& state) const
{
}

//------------------------------------------------------------------------
Snapshot Sample::saveSnapshot() const
{

}

//------------------------------------------------------------------------
void Sample::unload()
{

}

//------------------------------------------------------------------------
void Sample::setPosition(Nm point[3])
{
  for(int i = 0; i < 3; ++i) 
  {
    Nm d{m_bounds[2*i+1] - m_bounds[2*i]};

    m_bounds[2*i]   = point[i];
    m_bounds[2*i+1] = point[i] + d;
  }

}

//------------------------------------------------------------------------
void Sample::position(Nm point[3]) const
{
  for(int i = 0; i < 3; ++i) 
  {
    point[i] = m_bounds[2*i];
  }
}



// //------------------------------------------------------------------------
// Sample::Sample(const QString &id)
// : m_ID(id)
// {
//   memset(m_position, 0, 3*sizeof(double));
// 
//   vtkMath::UninitializeBounds(m_bounds);
// //   qDebug() << "Created Sample:" << id;
// }
// 
// //------------------------------------------------------------------------
// Sample::Sample(const QString &id, const QString &args)
// : m_ID(id)
// {
//   memset(m_position, 0, 3*sizeof(double));
// 
//   vtkMath::UninitializeBounds(m_bounds);
// //   qDebug() << "Created Sample:" << id;
// }
// 
// //------------------------------------------------------------------------
// Sample::~Sample()
// {
// //   qDebug() << data().toString() << ": Destructor";
// }
// 
// //------------------------------------------------------------------------
// void Sample::position(double pos[3])
// {
//   memcpy(pos, m_position, 3*sizeof(double));
// }
// 
// 
// //------------------------------------------------------------------------
// void Sample::setPosition(double pos[3])
// {
//   memcpy(m_position, pos, 3*sizeof(double));
// }
// 
// //------------------------------------------------------------------------
// void Sample::bounds(double value[6])
// {
//   memcpy(value, m_bounds, 6*sizeof(double));
// }
// 
// //------------------------------------------------------------------------
// void Sample::setBounds(double value[6])
// {
//   memcpy(m_bounds, value, 6*sizeof(double));
// }
// 
// //------------------------------------------------------------------------
// QVariant Sample::data(int role) const
// {
//   if (Qt::DisplayRole == role)
//     return m_ID;
//   else
//     return QVariant();
// }
// 
// //------------------------------------------------------------------------
// QString Sample::serialize() const
// {
// //   QString extensionArgs;
// //   foreach(ModelItemExtensionPtr ext, m_extensions)
// //   {
// //     SampleExtensionPtr sampleExt = sampleExtensionPtr(ext);
// //     QString serializedArgs = sampleExt->serialize(); //Independizar los argumentos?
// //     if (!serializedArgs.isEmpty())
// //       extensionArgs.append(ext->id()+"=["+serializedArgs+"];");
// //   }
// //   if (!extensionArgs.isEmpty())
// //   {
// //     m_args[EXTENSIONS] = QString("[%1]").arg(extensionArgs);
// //   }
//   return m_args.serialize();
// }
// 
// //------------------------------------------------------------------------
// void Sample::initialize(const Arguments &args)
// {
//   foreach(ArgumentId argId, args.keys())
//   {
//     if (argId != EXTENSIONS)
//       m_args[argId] = args[argId];
//   }
// }
// 
// //------------------------------------------------------------------------
// void Sample::initializeExtensions(const Arguments &args)
// {
// //   Arguments extArgs(args[EXTENSIONS]);
// //   foreach(ModelItemExtensionPtr ext, m_extensions)
// //   {
// //     SampleExtensionPtr sampleExt = sampleExtensionPtr(ext);
// //     qDebug() << extArgs;
// //     Arguments sArgs(extArgs.value(sampleExt->id(), QString()));
// //     qDebug() << sArgs;
// //     sampleExt->initialize(sArgs);
// //   }
// 
// }
// 
// //------------------------------------------------------------------------
// ChannelList Sample::channels()
// {
//   ChannelList channels;
// 
//   foreach(ModelItemSPtr item, relatedItems(EspINA::RELATION_OUT, Channel::STAIN_LINK))
//   {
//     channels << channelPtr(item.get());
//   }
// 
//   return channels;
// }
// 
// //------------------------------------------------------------------------
// SegmentationList Sample::segmentations()
// {
//   SegmentationList segmentations;
// 
//   ModelItemSList items = relatedItems(EspINA::RELATION_OUT, Relations::LOCATION);
//   while (!items.isEmpty())
//   {
//     ModelItemSPtr item = items.takeFirst();
//     if (EspINA::SEGMENTATION == item->type())
//     {
//       segmentations << segmentationPtr(item.get());
//     }
//     items << item->relatedItems(EspINA::RELATION_OUT, Relations::LOCATION);
//   }
// 
//   return segmentations;
// }
// 
// //------------------------------------------------------------------------
// SamplePtr EspINA::samplePtr(ModelItemPtr item)
// {
//   Q_ASSERT(SAMPLE == item->type());
//   SamplePtr ptr = dynamic_cast<SamplePtr>(item);
//   Q_ASSERT(ptr);
// 
//   return ptr;
// }
// 
// //------------------------------------------------------------------------
// SampleSPtr EspINA::samplePtr(ModelItemSPtr& item)
// {
//   Q_ASSERT(SAMPLE == item->type());
//   SampleSPtr ptr = boost::dynamic_pointer_cast<Sample>(item);
//   Q_ASSERT(ptr != NULL);
// 
//   return ptr;
// }
// 