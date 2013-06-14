/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "SegmentationNotes.h"



#include <Core/Model/EspinaModel.h>
#include <Core/Model/Filter.h>

using namespace EspINA;

const QString SegmentationNotes::EXTENSION_FILE = "SegmentationNotes/SegmentationNote";

const std::string FILE_VERSION = SegmentationNotesID.toStdString() + " 1.0\n";
const char SEP = ',';

SegmentationNotes::ExtensionCache SegmentationNotes::s_cache;

const Segmentation::InfoTag SegmentationNotes::NOTE  = "Notes";

//------------------------------------------------------------------------
SegmentationNotes::SegmentationNotes()
: m_loaded(false)
{
}

//------------------------------------------------------------------------
SegmentationNotes::~SegmentationNotes()
{
}

//------------------------------------------------------------------------
ModelItem::ExtId SegmentationNotes::id()
{
  return SegmentationNotesID;
}

//------------------------------------------------------------------------
Segmentation::InfoTagList SegmentationNotes::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << NOTE;

  return tags;

}

//------------------------------------------------------------------------
void SegmentationNotes::setSegmentation(SegmentationPtr seg)
{
  EspINA::Segmentation::Information::setSegmentation(seg);
}

//------------------------------------------------------------------------
QVariant SegmentationNotes::information(const Segmentation::InfoTag &tag)
{
  if (NOTE == tag)
  {
    loadNotesCache(m_segmentation);
    return s_cache[m_segmentation].Data.Note;
  }

  return QVariant();
}

//------------------------------------------------------------------------
QString SegmentationNotes::toolTipText() const
{
  loadNotesCache(m_segmentation);

  const QString WS  = "&nbsp;"; // White space
  const QString TAB = WS+WS+WS;

  QString toolTip;
  if (!s_cache[m_segmentation].Data.Note.isEmpty())
  {
    QString firstLine = s_cache[m_segmentation].Data.Note.left(20);
    if (firstLine.length() == 20)
      firstLine = firstLine.replace(17, 3, "...");
    toolTip = condition(":/espina/note.png", firstLine);
  }

  return toolTip;
}

//------------------------------------------------------------------------
void SegmentationNotes::loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() == FILE_VERSION)
  {
    char buffer[1024];
    while (file.readLine(buffer, sizeof(buffer)) > 0)
    {
      QString line(buffer);
      QStringList fields = line.split(SEP, QString::SkipEmptyParts);

      SegmentationPtr extensionSegmentation = NULL;
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
      {
        s_cache[extensionSegmentation].Data.Note = QString();
      }
    }
  }
}

//------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  bool invalid = false;

  if (seg->hasInformationExtension(SegmentationNotesID))
  {
    SegmentationNotes *extension = dynamic_cast<SegmentationNotes *>(
      seg->informationExtension(SegmentationNotesID));

    invalid = extension->note().isEmpty();
  }
  return invalid;
}

//------------------------------------------------------------------------
bool SegmentationNotes::saveCache(Snapshot &snapshot)
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

    loadNotesCache(segmentation);
    if (!data.Note.isEmpty())
    {
      QString file = QString("%1%2-%3.txt").arg(EXTENSION_FILE)
                                           .arg(segmentation->filter()->id())
                                           .arg(segmentation->outputId());

      snapshot << SnapshotEntry(file, data.Note.toUtf8());
    }
  }

  snapshot << SnapshotEntry(EXTENSION_FILE + ".csv", cache.str().c_str());

  return true;

}

//------------------------------------------------------------------------
Segmentation::InformationExtension SegmentationNotes::clone()
{
  return new SegmentationNotes();
}

//------------------------------------------------------------------------
void SegmentationNotes::initialize()
{
  s_cache.markAsClean(m_segmentation);
}

//------------------------------------------------------------------------
void SegmentationNotes::invalidate(SegmentationPtr segmentation)
{
  if (!segmentation)
    segmentation = m_segmentation;

  if (segmentation)
  {
    s_cache.markAsDirty(segmentation);
  }
}

//------------------------------------------------------------------------
void SegmentationNotes::setNote(const QString &note)
{
  s_cache[m_segmentation].Data.Note = note;
  m_segmentation->notifyModification(); // to update views
}

//------------------------------------------------------------------------
QString SegmentationNotes::note() const
{
  loadNotesCache(m_segmentation);

  return s_cache[m_segmentation].Data.Note;
}

//------------------------------------------------------------------------
void SegmentationNotes::loadNotesCache(SegmentationPtr segmentation) const
{
  if (!m_loaded)
  {
    ExtensionData &data = s_cache[segmentation].Data;

    if (data.Note.isEmpty())
    {
      QString segmentationFile = QString("%1%2-%3.txt").arg(EXTENSION_FILE)
      .arg(segmentation->filter()->id())
      .arg(segmentation->outputId());

      QFileInfo file(segmentation->filter()->cacheDir().absoluteFilePath(segmentationFile));

      if (file.exists())
      {
        QFile reader(file.absoluteFilePath());
        reader.open(QIODevice::ReadOnly);
        data.Note = reader.readAll();
      }
    }

    m_loaded = true;
  }
}
