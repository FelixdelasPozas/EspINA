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

// ESPINA
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/ViewItem.h>
#include <Core/Analysis/Extensible.hxx>

namespace ESPINA
{
  /** \brief Model biological structures which have been extracted from one or
   * more channels.
   */
  class EspinaCore_EXPORT Segmentation
  : public ViewItem
  , public Core::Analysis::Extensible<Core::SegmentationExtension, Segmentation>
  {
  public:
    /** \brief Segmentation class constructor.
     * \param[in] input input object smart pointer.
     *
     */
    explicit Segmentation(InputSPtr input);

    /** \brief Segmentation class destructor.
     *
     */
    virtual ~Segmentation();

    virtual void restoreState(const State& state);

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual void unload();

    /** \brief Sets an alternate name for the segmentation.
     * \param[in] alias alternate name.
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
     * \param[in] number numerical value.
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
     * \param[in] category category object smart pointer.
     *
     */
    void setCategory(CategorySPtr category);

    /** \brief Returns the category of the segmentation.
     *
     */
    CategorySPtr category() const
    { return m_category; }

    /** \brief Adds the user to the list of users that have modified this segmentation.
     * \param[in] user user name.
     *
     */
    void modifiedByUser(const QString& user)
    { m_users << user; }

    /** \brief Returns the list of users that have modified this segmentation.
     *
     */
    QStringList users() const
    { return m_users.toList(); }

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
    QString extensionPath(const Core::SegmentationExtensionSPtr extension) const
    { return extensionsPath() + extension->type() + "/"; }

    /** \brief Returns the path to save/load data files of a given extension and path.
     * \param[in] extension, segmentation extension object smart pointer.
     * \param[in] path, file path.
     *
     */
    QString extensionDataPath(const Core::SegmentationExtensionSPtr extension, QString path) const
    { return extensionPath(extension) + QString("%1_%2").arg(uuid()).arg(path); }

  private:
    QString                   m_alias;
    unsigned int              m_number;
    QSet<QString>             m_users;
    CategorySPtr              m_category;
  };
}
#endif // ESPINA_SEGMENTATION_H
