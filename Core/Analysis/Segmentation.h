/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "EspinaCore_Export.h"

#include "Core/Analysis/ViewItem.h"
#include "Core/Analysis/Extensions/SegmentationExtension.h"
#include "Core/Analysis/Persistent.h"
#include "Core/Analysis/Output.h"

namespace EspINA
{
  class EspinaCore_EXPORT Segmentation
  : public ViewItem
  , public Persistent
  {
    public:
      struct Existing_Extension{};

    public:
      explicit Segmentation(FilterSPtr filter, Output::Id output);
      virtual ~Segmentation();

      virtual void changeOutput(OutputSPtr output);

      virtual void restoreState(const State& state);

      virtual State saveState() const;

      virtual Snapshot saveSnapshot() const;

      virtual void unload();

      void setNumber(unsigned int number)      { m_number = number; }

      unsigned int number() const              { return m_number; }

      void setCategory(CategorySPtr category);
      CategorySPtr category() const            { return m_category; }

      void modifiedByUser(const QString& user) { m_users << user; }

      QStringList users() const                { return m_users.toList(); }

      /**
       * Extesion won't be available until requirements are satisfied
       */
      void addExtension(SegmentationExtensionSPtr extension) throw(SegmentationExtension::Existing_Extension);

      void deleteExtension(SegmentationExtensionSPtr extension) throw(SegmentationExtension::Extension_Not_Found);

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
      SegmentationExtensionSPtr extension(const SegmentationExtension::Type& type) const throw(SegmentationExtension::Extension_Not_Found);

      virtual SegmentationExtension::InfoTagList informationTags() const;

      virtual QVariant information(const SegmentationExtension::InfoTag& tag) const;

    private:
      unsigned int              m_number;
      QSet<QString>             m_users;
      CategorySPtr              m_category;
      SegmentationExtensionSMap m_extensions;
  };
}
#endif // ESPINA_SEGMENTATION_H
