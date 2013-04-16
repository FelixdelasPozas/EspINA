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


#include "TagExtension.h"
#include <Core/Model/EspinaModel.h>

using namespace EspINA;


const QString SegmentationTags::EXTENSION_FILE = "SegmentationTags/SegmentationTags.csv";

const std::string FILE_VERSION = SegmentationTagsID.toStdString() + " 1.0\n";
const char SEP = ',';

SegmentationTags::ExtensionCache SegmentationTags::s_cache;

QStringList SegmentationTags::s_availableTags;

QStringListModel SegmentationTags::TagModel;

const Segmentation::InfoTag SegmentationTags::TAGS  = "Tags";

//------------------------------------------------------------------------
SegmentationTags::SegmentationTags()
{
}

//------------------------------------------------------------------------
SegmentationTags::~SegmentationTags()
{
}

//------------------------------------------------------------------------
SegmentationTags *SegmentationTags::extension(SegmentationPtr segmentation)
{
  return dynamic_cast<SegmentationTags *>(segmentation->informationExtension(SegmentationTagsID));
}

//------------------------------------------------------------------------
ModelItem::ExtId SegmentationTags::id()
{
  return SegmentationTagsID;
}

//------------------------------------------------------------------------
Segmentation::InfoTagList SegmentationTags::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << TAGS;

  return tags;

}

//------------------------------------------------------------------------
void SegmentationTags::setSegmentation(SegmentationPtr seg)
{
  EspINA::Segmentation::Information::setSegmentation(seg);
}

//------------------------------------------------------------------------
QVariant SegmentationTags::information(const Segmentation::InfoTag &tag)
{
  if (TAGS == tag)
    return s_cache[m_segmentation].Data.Tags;

  return QVariant();
}

//------------------------------------------------------------------------
QString SegmentationTags::toolTipText() const
{
  QString toolTip;
  if (!tags().isEmpty())
    toolTip = condition(":/espina/tag.svg", tags().join(","));

  return toolTip;
}

//------------------------------------------------------------------------
void SegmentationTags::loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model)
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
          extensionSegmentation = segmentation.data();
        }
        i++;
      }
      if (extensionSegmentation)
      {
        ExtensionData &data = s_cache[extensionSegmentation].Data;
        for (int t = 2; t < fields.size(); ++t)
        {
          data.Tags << fields[t].trimmed().toLower();
        }
      }
    }
  }

  updateAvailableTags();
}

//------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  bool invalid = false;

  if (seg->hasInformationExtension(SegmentationTagsID))
  {
    SegmentationTags *extension = dynamic_cast<SegmentationTags *>(
      seg->informationExtension(SegmentationTagsID));

    invalid = extension->tags().isEmpty();
  }

  return invalid;
}

//------------------------------------------------------------------------
bool SegmentationTags::saveCache(Snapshot &snapshot)
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

    cache << SEP << data.Tags.join(",").toStdString();

    cache << std::endl;
  }

  snapshot << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());

  return true;

}

//------------------------------------------------------------------------
Segmentation::InformationExtension SegmentationTags::clone()
{
  return new SegmentationTags();
}

//------------------------------------------------------------------------
void SegmentationTags::initialize()
{
  s_cache.markAsClean(m_segmentation);
}

//------------------------------------------------------------------------
void SegmentationTags::invalidate(SegmentationPtr segmentation)
{
  if (!segmentation)
    segmentation = m_segmentation;

  if (segmentation)
  {
    s_cache.markAsDirty(segmentation);
  }
}

//------------------------------------------------------------------------
void SegmentationTags::addTag(const QString &tag)
{
  s_cache.markAsClean(m_segmentation);

  addTagImplementation(tag.toLower());

  s_cache[m_segmentation].Data.Tags.sort();

  updateAvailableTags();
}

//------------------------------------------------------------------------
void SegmentationTags::addTags(const QStringList &tags)
{
  s_cache.markAsClean(m_segmentation);

  foreach(QString tag, tags)
  {
    addTagImplementation(tag.toLower());
  }

  s_cache[m_segmentation].Data.Tags.sort();

  updateAvailableTags();
}

//------------------------------------------------------------------------
void SegmentationTags::removeTag(const QString &tag)
{
  s_cache.markAsClean(m_segmentation);

  QStringList &currentTags = s_cache[m_segmentation].Data.Tags;
  if (currentTags.contains(tag))
  {
    currentTags.removeOne(tag);
  }

  updateAvailableTags();
}

//------------------------------------------------------------------------
void SegmentationTags::setTags(const QStringList &tags)
{
  s_cache[m_segmentation].Data.Tags.clear(); 
  addTags(tags);
}

//------------------------------------------------------------------------
void SegmentationTags::addTagImplementation(const QString &tag)
{
  QStringList &currentTags = s_cache[m_segmentation].Data.Tags;
  if (!currentTags.contains(tag))
  {
    currentTags << tag.trimmed();
  }
}

//------------------------------------------------------------------------
void SegmentationTags::updateAvailableTags()
{
  QStringList tags;

  foreach(CacheEntry<ExtensionData> entry, s_cache)
  {
    if (!entry.Dirty)
    {
      tags << entry.Data.Tags;
    }
  }

  tags.removeDuplicates();

  if (tags.toSet() != s_availableTags.toSet())
  {
    s_availableTags = tags;
    TagModel.setStringList(s_availableTags);
  }
}

