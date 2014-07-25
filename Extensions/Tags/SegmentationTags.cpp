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


#include "SegmentationTags.h"
#include <GUI/Utils/Conditions.h>
#include <Core/Analysis/Segmentation.h>

using namespace ESPINA;


const QString                        SegmentationTags::TYPE = "SegmentationTags";
const SegmentationExtension::InfoTag SegmentationTags::TAGS = "Tags";

//------------------------------------------------------------------------
SegmentationTags::SegmentationTags(const InfoCache &infoCache)
: SegmentationExtension(infoCache)
{
}

//------------------------------------------------------------------------
SegmentationTags::~SegmentationTags()
{
}

//------------------------------------------------------------------------
void SegmentationTags::onExtendedItemSet(Segmentation* item)
{

}

//------------------------------------------------------------------------
SegmentationExtension::InfoTagList SegmentationTags::availableInformations() const
{
  InfoTagList tags;

  tags << TAGS;

  return tags;
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
void SegmentationTags::addTag(const QString &tag)
{
  addTagImplementation(tag.toLower());

  m_tags.sort();

  updateAvailableTags();
}

//------------------------------------------------------------------------
void SegmentationTags::addTags(const QStringList &tags)
{
  for(auto tag : tags)
  {
    addTagImplementation(tag.toLower());
  }

  m_tags.sort();

  updateAvailableTags();
}

//------------------------------------------------------------------------
void SegmentationTags::removeTag(const QString &tag)
{
  if (m_tags.contains(tag))
  {
    m_tags.removeOne(tag);
  }

  updateAvailableTags();
}

//------------------------------------------------------------------------
void SegmentationTags::setTags(const QStringList &tags)
{
  m_tags.clear(); 

  addTags(tags);
}

//------------------------------------------------------------------------
QVariant SegmentationTags::cacheFail(const QString& tag) const
{
  return (TAGS == tag)?state():QVariant();
}

//------------------------------------------------------------------------
void SegmentationTags::addTagImplementation(const QString &tag)
{
  if (!m_tags.contains(tag))
  {
    m_tags << tag.trimmed();
  }
}

//------------------------------------------------------------------------
void SegmentationTags::updateAvailableTags()
{

}

// //------------------------------------------------------------------------
// void SegmentationTags::updateAvailableTags()
// {
//   QStringList tags;
// 
//   foreach(CacheEntry<ExtensionData> entry, s_cache)
//   {
//     if (!entry.Dirty)
//     {
//       tags << entry.Data.Tags;
//     }
//   }
// 
//   tags.removeDuplicates();
// 
//   if (tags.toSet() != s_availableTags.toSet())
//   {
//     s_availableTags = tags;
//     TagModel.setStringList(s_availableTags);
//   }
// }