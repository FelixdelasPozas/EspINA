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

#ifndef ESPINA_TAG_EXTENSION_H
#define ESPINA_TAG_EXTENSION_H

#include "Extensions/EspinaExtensions_Export.h"

#include <Core/Analysis/Extensions/SegmentationExtension.h>
#include <GUI/Model/SegmentationAdapter.h>

#include <QStringListModel>

namespace EspINA
{


  class EspinaExtensions_EXPORT SegmentationTags
  : public SegmentationExtension
  {
    const static QString FILE;

  public:
    static const InfoTag TAGS;
    static const Type    TYPE;

    //static QStringListModel TagModel;
  public:
    explicit SegmentationTags();
    virtual ~SegmentationTags();

    virtual Type type() const
    { return TYPE; }

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual void onSegmentationSet(SegmentationPtr seg);

    virtual bool validCategory(const QString& classificationName) const
    { return true; }

    virtual InfoTagList availableInformations() const;

    virtual QVariant information(const InfoTag& tag) const;

    virtual QString toolTipText() const;

    void addTag(const QString &tag);

    void addTags(const QStringList &tags);

    void removeTag(const QString &tag);

    void setTags(const QStringList &tags);

    QStringList tags() const
    { return m_tags; }

  private:
    void addTagImplementation(const QString &tag);

    void updateAvailableTags();

    QStringList m_tags;
  };

  using SegmentationTagsPtr  = SegmentationTags *;
  using SegmentationTagsSPtr = std::shared_ptr<SegmentationTags>;

  SegmentationTagsSPtr EspinaExtensions_EXPORT tagsExtension(SegmentationAdapterPtr segmentation);

} // namespace EspINA

#endif // TAGEXTENSION_H
