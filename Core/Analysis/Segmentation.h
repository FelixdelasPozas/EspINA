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

//----------------------------------------------------------------------------
// File:    Segmentation.h
// Purpose: Model biological structures which have been extracted from one or
//          more channels.
//----------------------------------------------------------------------------
#ifndef ESPINA_SEGMENTATION_H
#define ESPINA_SEGMENTATION_H

#include "Core/EspinaCore_Export.h"

#include "Core/Analysis/ViewItem.h"
#include "Core/Analysis/Extension.h"

namespace EspINA
{
  class EspinaCore_EXPORT Segmentation
  : public ViewItem
  {
  public:
    struct Existing_Extension{};

  public:
    explicit Segmentation(InputSPtr input);
    virtual ~Segmentation();

    virtual void restoreState(const State& state);

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual void unload();

    void setAlias(const QString& alias)
    { m_alias = alias; }

    QString alias() const
    { return m_alias; }

    void setNumber(unsigned int number)
    { m_number = number; }

    unsigned int number() const
    { return m_number; }

    void setCategory(CategorySPtr category);

    CategorySPtr category() const
    { return m_category; }

    void modifiedByUser(const QString& user)
    { m_users << user; }

    QStringList users() const
    { return m_users.toList(); }

    /**
     * Extesion won't be available until requirements are satisfied
     */
    void addExtension(SegmentationExtensionSPtr extension)
      throw(SegmentationExtension::Existing_Extension);

    void deleteExtension(SegmentationExtensionSPtr extension)
      throw(SegmentationExtension::Extension_Not_Found);

    /** \brief Check whether or not there is an extension with the given name
     *
     */
    bool hasExtension(const SegmentationExtension::Type& type) const;

    /** \brief Return the extension with the especified name
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

    SegmentationExtensionSList extensions() const
    { return m_extensions.values(); }

    virtual SegmentationExtension::InfoTagList informationTags() const;

    virtual QVariant information(const SegmentationExtension::InfoTag& tag) const;

    bool isInformationReady(const SegmentationExtension::InfoTag &tag) const;

  private:
    QString extensionsPath() const
    { return "Extensions/"; }

    QString extensionPath(const SegmentationExtensionSPtr extension) const
    { return extensionsPath() + extension->type() + "/"; }

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
