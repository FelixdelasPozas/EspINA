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

#ifndef ESPINA_SEGMENTATION_TAGS
#define ESPINA_SEGMENTATION_TAGS

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Extension.h>
#include <GUI/Model/SegmentationAdapter.h>

// Qt
#include <QStringListModel>

namespace ESPINA
{
  class EspinaExtensions_EXPORT SegmentationTags
  : public SegmentationExtension
  {
  public:
    static const InfoTag TAGS;
    static const Type    TYPE;

    //static QStringListModel TagModel;
  public:
    /* \brief SegmentationTags class constructor.
     * \param[in] infoCache, cache object.
     *
     */
    explicit SegmentationTags(const InfoCache& infoCache = InfoCache());

    /* \brief SegmentationTags class virtual destructor.
     *
     */
    virtual ~SegmentationTags();

    /* \brief Implements Extension::type().
     *
     */
    virtual Type type() const
    { return TYPE; }

    /* \brief Implements Extension::invalidateOnChange().
     *
     */
    virtual bool invalidateOnChange() const
    { return false; }

    /* \brief Implements Extension::state().
     *
     */
    virtual State state() const
    { return m_tags.join(","); }

    /* \brief Implements Extension::snapshot().
     *
     */
    virtual Snapshot snapshot() const
    { return Snapshot(); }

    /* \brief Implements Extension::dependencies().
     *
     */
    virtual TypeList dependencies() const
    { return TypeList(); }

    /* \brief Implements SegmentationExtension::validCategory().
     *
     */
    virtual bool validCategory(const QString& classificationName) const
    { return true; }

    /* \brief Implements Extension::availableInformations().
     *
     */
    virtual InfoTagList availableInformations() const;

    /* \brief Overrides Extension::tooltipText() const.
     *
     */
    virtual QString toolTipText() const override;

    /* \brief Adds a tag.
     * \param[in] tag, text string.
     *
     */
    void addTag(const QString &tag);

    /* \brief Adds multiple tags.
     * \param[in] tags, text string list.
     *
     */
    void addTags(const QStringList &tags);

    /* \brief Removes a tag.
     * \param[in] tag, text string.
     *
     */
    void removeTag(const QString &tag);

    /* \brief Sets the tags.
     * \param[in] tags, text string list.
     *
     */
    void setTags(const QStringList &tags);

    /* \brief Returns the tags.
     *
     */
    QStringList tags() const
    { return m_tags; }

  protected:
    /* \brief Implements Extension::OnExtendedItemSet().
     *
     */
    virtual void onExtendedItemSet(Segmentation* item);

    /* \brief Implements Extension::cacheFail().
     *
     */
    virtual QVariant cacheFail(const QString& tag) const;

  private:
    /* \brief Returns trimmed tag (spaces removed at the beginning and end of the string).
     *
     */
    void addTagImplementation(const QString &tag);

    /* \brief Updates available tags for all extensions.
     *
     */
    void updateAvailableTags();

    QStringList m_tags;
    static QStringList s_availableTags;
  };

  using SegmentationTagsPtr  = SegmentationTags *;
  using SegmentationTagsSPtr = std::shared_ptr<SegmentationTags>;

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_TAGS
