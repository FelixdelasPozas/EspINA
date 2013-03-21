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


#ifndef TAGEXTENSION_H
#define TAGEXTENSION_H

#include <Core/Extensions/SegmentationExtension.h>

namespace EspINA
{

  const ModelItem::ExtId TagExtensionID = "TagsExtension";

  class SegmentationTags
  : public Segmentation::Information
  {
    struct ExtensionData
    {
      QStringList Tags;
    };

    typedef Cache<SegmentationPtr, ExtensionData> ExtensionCache;

    static ExtensionCache s_cache;

    const static QString EXTENSION_FILE;

  public:
    explicit SegmentationTags();
    virtual ~SegmentationTags();

    virtual Segmentation::ExtId id();

    virtual Segmentation::ExtIdList dependencies() const
    { return Segmentation::Extension::dependencies(); }

    virtual Segmentation::InfoTagList availableInformations() const;

    virtual bool validTaxonomy(const QString &qualifiedName) const
    { return true; }

    virtual QVariant information(const Segmentation::InfoTag &tag);

    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }

    virtual void loadCache(QuaZipFile &file,
                           const QDir &tmpDir,
                           EspinaModel *model);

    virtual bool saveCache(Snapshot &snapshot);

    virtual Segmentation::InformationExtension clone();

    virtual void initialize();

    virtual void invalidate(SegmentationPtr segmentation = 0);

    void addTag(const QString &tag);

    void addTags(const QStringList &tags);

    void removeTag(const QString &tag);

    void setTags(const QStringList &tags);

    QStringList tags() const
    { return s_cache[m_segmentation].Data.Tags; }

  };

} // namespace EspINA

#endif // TAGEXTENSION_H
