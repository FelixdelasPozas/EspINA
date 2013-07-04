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


#include "VisualizationState.h"

#include "Core/Model/Segmentation.h"
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Filter.h>

// ITK
#include <itkShapeLabelObject.h>

// Qt
#include <QApplication>
#include <QDebug>

using namespace EspINA;


const QString VisualizationState::EXTENSION_FILE = "VisualizationState/VisualizationState.cfg";

const std::string FILE_VERSION = VisualizationStateID.toStdString() + " 1.0\n";
const char SEP = ',';

VisualizationState::ExtensionCache VisualizationState::s_cache;

//------------------------------------------------------------------------
VisualizationState::ExtensionData::ExtensionData()
{
}

//------------------------------------------------------------------------
VisualizationState::VisualizationState()
{
}

//------------------------------------------------------------------------
VisualizationState::~VisualizationState()
{
  if (m_segmentation)
  {
    //qDebug() << m_seg->data().toString() << ": Deleting" << VisualizationStateID;
    invalidate(m_segmentation);
  }
}

//------------------------------------------------------------------------
ModelItem::ExtId VisualizationState::id()
{
  return VisualizationStateID;
}


//------------------------------------------------------------------------
Segmentation::InfoTagList VisualizationState::availableInformations() const
{
  Segmentation::InfoTagList tags;

  return tags;
}

//------------------------------------------------------------------------
void VisualizationState::setSegmentation(SegmentationPtr seg)
{
  Segmentation::Information::setSegmentation(seg);

//   connect(m_segmentation, SIGNAL(outputModified()),
//           this, SLOT(invalidate()));
// 
//   if (m_segmentation->outputIsModified())
//     invalidate();
//   else
    initialize();
}

//------------------------------------------------------------------------
QVariant VisualizationState::information(const Segmentation::InfoTag &tag)
{
  qWarning() << VisualizationStateID << ":"  << tag << " is not provided";
  return QVariant();
}

//------------------------------------------------------------------------
void VisualizationState::loadCache(QuaZipFile   &file,
                                   const QDir   &tmpDir,
                                   IEspinaModel *model)
{
  enum { SEG_ID, REP_LABEL, REP_SETTINGS } state;

  QString header(file.readLine());
  if (header.toStdString() == FILE_VERSION)
  {
    char buffer[1024];

    state = SEG_ID;

    QString repLabel;
    SegmentationPtr extensionSegmentation = NULL;
    while (file.readLine(buffer, sizeof(buffer)) > 0)
    {
      QString line(buffer);
      switch (state)
      {
        case SEG_ID:
        {
          QStringList fields = line.split(SEP);

          extensionSegmentation = NULL;
          int i = 0;
          while (!extensionSegmentation && i < model->segmentations().size())
          {
            SegmentationSPtr segmentation = model->segmentations()[i];
            if ( segmentation->filter()->id()       == fields[0]
              && segmentation->outputId()           == fields[1].toInt()
              && segmentation->filter()->cacheDir() == tmpDir)
            {
              extensionSegmentation = segmentation.get();
            }
            i++;
          }
          if (extensionSegmentation)
            state = REP_LABEL;
        }
        break;
        case REP_LABEL:
        {
          if (line == "\n")
          {
            extensionSegmentation->output()->updateModificationTime();
            state = SEG_ID;
          } else 
          {
            repLabel = line.trimmed();
            state = REP_SETTINGS;
          }
        }
        break;
        case REP_SETTINGS:
        {
          ExtensionData &data = s_cache[extensionSegmentation].Data;
          data.Settings[repLabel] = line.trimmed();
          state = REP_LABEL;
        }
        break;
      }
    };
  }
}

//------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  bool invalid = false;
  if (seg->hasInformationExtension(VisualizationStateID))
  {
    invalid = !seg->informationExtension(VisualizationStateID)->isEnabled();
//   } else 
//   {
//     invalid = seg->outputIsModified();
  }
  return invalid;
}

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
bool VisualizationState::saveCache(Snapshot &snapshot)
{
  s_cache.purge(invalidData);

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  foreach(SegmentationPtr segmentation, s_cache.keys())
  {
    ExtensionData &data = s_cache[segmentation].Data;

    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();
    cache << std::endl;

    foreach(QString key, data.Settings.keys())
    {
      cache << key.toStdString() << std::endl;
      cache << data.Settings[key].toStdString() << std::endl;
    }

    cache << std::endl;
  }

  snapshot << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//------------------------------------------------------------------------
Segmentation::InformationExtension VisualizationState::clone()
{
  return new VisualizationState();
}

//------------------------------------------------------------------------
void VisualizationState::initialize()
{
  s_cache.markAsClean(m_segmentation);
}

//------------------------------------------------------------------------
void VisualizationState::invalidate(SegmentationPtr segmentation)
{
  if (!segmentation)
    segmentation = m_segmentation;

  if (segmentation)
  {
    //qDebug() << "Invalidate" << m_seg->data().toString() << VisualizationStateID;
    s_cache.markAsDirty(segmentation);
  }
}

//------------------------------------------------------------------------
void VisualizationState::setSettings(QString key, QString settings)
{
  s_cache[m_segmentation].Data.Settings[key] = settings;
}

//------------------------------------------------------------------------
QString VisualizationState::settings(QString key)
{
  return s_cache[m_segmentation].Data.Settings.value(key, QString());
}
