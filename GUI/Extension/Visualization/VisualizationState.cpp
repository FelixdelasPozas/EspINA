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


#include "VisualizationState.h"

// ITK
#include <itkShapeLabelObject.h>

// Qt
#include <QApplication>
#include <QDebug>

using namespace ESPINA;

const SegmentationExtension::Type VisualizationState::TYPE = "VisualizationState";

const std::string EXTENSION_VERSION = "1.0\n";

const char SEP = ',';

//------------------------------------------------------------------------
VisualizationState::VisualizationState()
: SegmentationExtension(InfoCache())
{
}

//------------------------------------------------------------------------
VisualizationState::~VisualizationState()
{
}

//------------------------------------------------------------------------
void VisualizationState::onSegmentationSet(SegmentationPtr seg)
{
}

//------------------------------------------------------------------------
SegmentationExtension::InfoTagList VisualizationState::availableInformations() const
{
  InfoTagList tags;

  return tags;
}

//------------------------------------------------------------------------
QVariant VisualizationState::information(const InfoTag &tag) const
{
  qWarning() << TYPE << " Extension:"  << tag << " is not provided";
  return QVariant();
}

// //------------------------------------------------------------------------
// void VisualizationState::loadCache(QuaZipFile   &file,
//                                    const QDir   &tmpDir,
//                                    IEspinaModel *model)
// {
//   enum { SEG_ID, REP_LABEL, REP_SETTINGS } state;
// 
//   QString header(file.readLine());
//   if (header.toStdString() == FILE_VERSION)
//   {
//     char buffer[1024];
// 
//     state = SEG_ID;
// 
//     QString repLabel;
//     SegmentationPtr extensionSegmentation = NULL;
//     while (file.readLine(buffer, sizeof(buffer)) > 0)
//     {
//       QString line(buffer);
//       switch (state)
//       {
//         case SEG_ID:
//         {
//           QStringList fields = line.split(SEP);
// 
//           extensionSegmentation = NULL;
//           int i = 0;
//           while (!extensionSegmentation && i < model->segmentations().size())
//           {
//             SegmentationSPtr segmentation = model->segmentations()[i];
//             if ( segmentation->filter()->id()       == fields[0]
//               && segmentation->outputId()           == fields[1].toInt()
//               && segmentation->filter()->cacheDir() == tmpDir)
//             {
//               extensionSegmentation = segmentation.get();
//             }
//             i++;
//           }
//           if (extensionSegmentation)
//             state = REP_LABEL;
//         }
//         break;
//         case REP_LABEL:
//         {
//           if (line == "\n")
//           {
//             extensionSegmentation->output()->updateModificationTime();
//             state = SEG_ID;
//           } else
//           {
//             repLabel = line.trimmed();
//             state = REP_SETTINGS;
//           }
//         }
//         break;
//         case REP_SETTINGS:
//         {
//           ExtensionData &data = s_cache[extensionSegmentation].Data;
//           data.Settings[repLabel] = line.trimmed();
//           state = REP_LABEL;
//         }
//         break;
//       }
//     };
//   }
// }


// File Format:
// version
// filterId, outputId
// representation label 1
// settings 1
// ..
// representation label n
// settings n
// >empty line< // empty lines mark the end of a segmentation
// filterId, outputId
// representation label 1
// settings 1
// ..
// representation label n
// settings n
// >empty line< // empty lines mark the end of a segmentation
//------------------------------------------------------------------------
// bool VisualizationState::saveCache(Snapshot &snapshot)
// {
//   s_cache.purge(invalidData);
// 
//   if (s_cache.isEmpty())
//     return false;
// 
//   std::ostringstream cache;
//   cache << FILE_VERSION;
// 
//   foreach(SegmentationPtr segmentation, s_cache.keys())
//   {
//     ExtensionData &data = s_cache[segmentation].Data;
// 
//     cache << segmentation->filter()->id().toStdString();
//     cache << SEP << segmentation->outputId();
//     cache << std::endl;
// 
//     foreach(QString key, data.Settings.keys())
//     {
//       cache << key.toStdString() << std::endl;
//       cache << data.Settings[key].toStdString() << std::endl;
//     }
// 
//     cache << std::endl;
//   }
// 
//   snapshot << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());
// 
//   return true;
// }

//------------------------------------------------------------------------
void VisualizationState::setState(const QString& representation, const QString& state)
{
  m_state[representation] = state;
}

//------------------------------------------------------------------------
QString VisualizationState::state(const QString& representation)
{
  return m_state.value(representation, QString());
}