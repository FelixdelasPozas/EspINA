/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_SEGMENTATION_H
#define ESPINA_SEGMENTATION_H

#include "Core/EspinaCore_Export.h"

#include "Core/Analysis/ViewItem.h"
#include "Core/Analysis/Extension.h"

namespace ESPINA
{
	/** \brief Model biological structures which have been extracted from one or
	 * more channels.
	 */
  class EspinaCore_EXPORT Segmentation
  : public ViewItem
  {
  public:
    struct Existing_Extension{};

  public:
    /** \brief Segmentation class constructor.
     * \param[in] input, input object smart pointer.
     *
     */
    explicit Segmentation(InputSPtr input);

    /** \brief Segmentation class destructor.
     *
     */
    virtual ~Segmentation();

    /** \brief Implements Persisten::restoreState().
     *
     */
    virtual void restoreState(const State& state);

    /** \brief Implements Persistent::state() const.
     *
     */
    virtual State state() const;

    /** \brief Implements Persistent::snapshot() const.
     *
     */
    virtual Snapshot snapshot() const;

    /** \brief Implements Persisten::unload().
     *
     */
    virtual void unload();

    /** \brief Sets an alternate name for the segmentation.
     * \param[in] alias, alternate name.
     *
     */
    void setAlias(const QString& alias)
    { m_alias = alias; }

    /** \brief Returns the alternate name for the segmentation.
     *
     */
    QString alias() const
    { return m_alias; }

    /** \brief Sets the number of the segmentation.
     * \param[in] number, numerical value.
     *
     */
    void setNumber(unsigned int number)
    { m_number = number; }

    /** \brief Returns the number of the segmentation.
     *
     */
    unsigned int number() const
    { return m_number; }

    /** \brief Sets the category of the segmentation.
     * \param[in] category, category object smart pointer.
     *
     */
    void setCategory(CategorySPtr category);

    /** \brief Returns the category of the segmentation.
     *
     */
    CategorySPtr category() const
    { return m_category; }

    /** \brief Adds the user to the list of users that have modified this segmentation.
     * \param[in] user, user name.
     *
     */
    void modifiedByUser(const QString& user)
    { m_users << user; }

    /** \brief Returns the list of users that have modified this segmentation.
     *
     */
    QStringList users() const
    { return m_users.toList(); }

    /** \brief Adds a extension to this segmentation.
     * \param[in] extension, segmentation extension object smart pointer.
     *
     * Extesion won't be available until requirements are satisfied
     *
     */
    void addExtension(SegmentationExtensionSPtr extension)
      throw(SegmentationExtension::Existing_Extension);

    /** \brief Removes an extension from the list of extenions.
     * \param[in] extension, segmentation extension object smart pointer.
     *
     */
    void deleteExtension(SegmentationExtensionSPtr extension)
      throw(SegmentationExtension::Extension_Not_Found);

    /** \brief Check whether or not there is an extension with the given type.
     * \param[in] type, segmentation extension type.
     *
     */
    bool hasExtension(const SegmentationExtension::Type& type) const;

    /** \brief Return the extension with the especified type.
     * \param[in] type, segmentation extension type.
     *
     *  Important: It the segmentation doesn't contain any extension with
     *  the requested name, but there exist an extension prototype registered
     *  in the factory, a new instance will be created and attached to the
     *  segmentation.
     *  If there is no extension with the given name registered in the factory
     *  a Undefined_Extension exception will be thrown
     */
    SegmentationExtensionSPtr extension(const SegmentationExtension::Type& type) const
      throw(SegmentationExtension::Extension_Not_Found);

    /** \brief Returns a list of segmentation extension smart pointers.
     *
     */
    SegmentationExtensionSList extensions() const
    { return m_extensions.values(); }

    /** \brief Returns the list of information tag this segmentation can provide.
     *
     */
    virtual SegmentationExtension::InfoTagList informationTags() const;

    /** \brief Returns the value of the specified information tag.
     * \param[in] tag, information key.
     *
     */
    virtual QVariant information(const SegmentationExtension::InfoTag& tag) const;

    /** \brief Returns true if the information has been generated.
     * \param[in] tag, information key.
     *
     */
    bool isInformationReady(const SegmentationExtension::InfoTag &tag) const;

  private:
    /** \brief Returns the path to save/load extensions data files.
     *
     */
    QString extensionsPath() const
    { return "Extensions/"; }

    /** \brief Returns the path to save/load data files of a given extension.
     * \param[in] extension, segmentation extension object smart pointer.
     *
     */
    QString extensionPath(const SegmentationExtensionSPtr extension) const
    { return extensionsPath() + extension->type() + "/"; }

    /** \brief Returns the path to save/load data files of a given extension and path.
     * \param[in] extension, segmentation extension object smart pointer.
     * \param[in] path, file path.
     *
     */
    QString extensionDataPath(const SegmentationExtensionSPtr extension, QString path) const
    { return extensionPath(extension) + QString("%1_%2").arg(uuid()).arg(path); }

  private:
    QString                   m_alias;
    unsigned int              m_number;
    QSet<QString>             m_users;
    CategorySPtr              m_category;
    SegmentationExtensionSMap m_extensions;
  };
}
#endif // ESPINA_SEGMENTATION_H
