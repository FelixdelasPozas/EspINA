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

// ESPINA
#include "SegmentationTags.h"
#include <GUI/Utils/Format.h>
#include <Core/Analysis/Segmentation.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI::Utils::Format;

const SegmentationExtension::Type SegmentationTags::TYPE = "SegmentationTags";
const SegmentationExtension::Key  SegmentationTags::TAGS = "Tags";

QMap<QString, unsigned int> SegmentationTags::s_availableTags;
QReadWriteLock              SegmentationTags::s_mutex;

//------------------------------------------------------------------------
SegmentationTags::SegmentationTags(const InfoCache &infoCache)
: SegmentationExtension{infoCache}
{
  for (auto tag : infoCache[TAGS].toString().split(";"))
  {
    addTagImplementation(tag);
  }
}

//------------------------------------------------------------------------
SegmentationTags::~SegmentationTags()
{
  if(!m_tags.empty())
  {
    for(auto tag: m_tags)
    {
      removeFromAvailableTags(tag);
    }
  }
}

//------------------------------------------------------------------------
void SegmentationTags::onExtendedItemSet(Segmentation* item)
{
}

//------------------------------------------------------------------------
const SegmentationExtension::InformationKeyList SegmentationTags::availableInformation() const
{
  InformationKeyList keys;

  keys << createKey(TAGS);

  return keys;
}

//------------------------------------------------------------------------
const QString SegmentationTags::toolTipText() const
{
  QString toolTip;
  auto tagList = tags();
  if (!tagList.isEmpty())
  {
    tagList.removeDuplicates();
    toolTip = createTable(":/espina/tag.svg", tagList.join(","));
  }

  return toolTip;
}

//------------------------------------------------------------------------
void SegmentationTags::addTag(const QString &tag)
{
  addTagImplementation(tag.toLower());

  m_tags.sort();

  updateInfoCache(TAGS, state());
}

//------------------------------------------------------------------------
void SegmentationTags::addTags(const QStringList &tags)
{
  for(auto tag : tags)
  {
    addTagImplementation(tag.toLower());
  }

  m_tags.sort();

  updateInfoCache(TAGS, state());
}

//------------------------------------------------------------------------
void SegmentationTags::removeTag(const QString &tag)
{
  if (m_tags.contains(tag))
  {
    m_tags.removeOne(tag);

    removeFromAvailableTags(tag);

    updateInfoCache(TAGS, state());
  }
}

//------------------------------------------------------------------------
void SegmentationTags::setTags(const QStringList &tags)
{
  for(auto tag: m_tags)
  {
    removeFromAvailableTags(tag);
  }

  m_tags.clear();

  addTags(tags);
}

//------------------------------------------------------------------------
QVariant SegmentationTags::cacheFail(const InformationKey& key) const
{
  return (TAGS == key.value())? state() : QVariant();
}

//------------------------------------------------------------------------
void SegmentationTags::addTagImplementation(const QString &tag)
{
  if (!tag.isEmpty())
  {
    m_tags << tag.trimmed();

    addToAvailableTags(tag.trimmed());
  }
}

//------------------------------------------------------------------------
QStringList SegmentationTags::availableTags()
{
  QReadLocker lock(&s_mutex);

  return s_availableTags.keys();
}

//------------------------------------------------------------------------
void SegmentationTags::addToAvailableTags(const QString& tag)
{
  QWriteLocker lock(&s_mutex);

  if(s_availableTags.keys().contains(tag))
  {
    s_availableTags[tag] += 1;
  }
  else
  {
    s_availableTags.insert(tag, 1);
  }
}

//------------------------------------------------------------------------
void SegmentationTags::removeFromAvailableTags(const QString& tag)
{
  QWriteLocker lock(&s_mutex);

  Q_ASSERT(s_availableTags.keys().contains(tag));

  s_availableTags[tag] -= 1;

  if(s_availableTags[tag] == 0)
  {
    s_availableTags.remove(tag);
  }
}

//------------------------------------------------------------------------
QStringList SegmentationTags::tags() const
{
  return m_tags;
}
