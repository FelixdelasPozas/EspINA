/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_CHANNEL_H
#define ESPINA_CHANNEL_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/ViewItem.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Extensible.hxx>

namespace ESPINA
{
  class EspinaCore_EXPORT Channel
  : public ViewItem
  , public Core::Analysis::Extensible<Core::StackExtension, Channel>
  {
    public:
      static const RelationName STAIN_LINK;

    public:
      /** \brief Channel class constructor.
       * \param[in] input, input object smart pointer.
       *
       */
      explicit Channel(InputSPtr input);

      /** \brief Channel class destructor.
       *
       */
      virtual ~Channel();

      virtual void restoreState(const State& state);

      virtual State state() const;

      virtual Snapshot snapshot() const;

      virtual void unload();

      /** \brief Sets the position of the channel.
       * \param[in] point, origin point of the channel.
       *
       */
      void setPosition(const NmVector3& point);

      /** \brief Returns the position (origin) of the channel.
       *
       */
      NmVector3 position() const;

      /** \brief Set channel's hue.
       *
       *  Hue value belongs to [0,1] U -1.\n
       *  Not stained channels hue value is -1
       */
      void setHue(double hue);

      /** \brief Channel's hue.
       *
       *  Hue value belongs to [0,1] U -1.\n
       *  Not stained channels hue value is -1
       */
      double hue() const
      { return m_hue; }

      /** \brief Set channel's opacity.
       *
       * Opacity value belong to [0,1] U -1 \n
       * -1 value means opacity is automatically managed
       */
      void setOpacity(double opacity);

      /** \brief Channel's opacity.
       *
       * Opacity value belong to [0,1] U -1 \n
       * -1 value means opacity is automatically managed
       */
      double opacity() const
      { return m_opacity; }

      /** \brief Set channel's saturation.
       *
       *  Saturation value belongs to [0,1].
       */
      void setSaturation(double saturation);

      /** \brief Channel's saturation.
       *
       *  Saturation value belongs to [0,1]
       */
      double saturation() const
      { return m_saturation; }

      /** \brief Set channel's contrast.
       *
       *  Contrast value belongs to [0,2]
       */
      void setContrast(double contrast);

      /** \brief Channel's contrast.
       *
       *  Contrast value belongs to [0,2]
       */
      double contrast() const
      { return m_contrast; }

      /** \brief Set channel's brightness.
       *
       *  Brightness value belongs to [-1,1]
       */
      void setBrightness(double brightness);

      /** \brief Channel's brightness.
       *
       *  Brightness value belongs to [-1,1]
       */
      double brightness() const
      { return m_brightness; }

      /** \brief Set channel's metadata.
       *
       */
      void setMetadata(const QString& metadata);

      /** \brief Returns channel's metadata.
       *
       */
      QString metadata() const;

      static const int AUTOMATIC_OPACITY = -1;

    private:
      /** \brief Returns the metadata file name for this channel.
       *
       */
      QString metadataFile() const
      { return QString("Metadata/%1/metadata.xml").arg(uuid()); }

      /** \brief Returns the path of the extensions for this channel.
       *
       */
      QString extensionsPath() const
      { return "Extensions/"; }

      /** \brief Returns the path of the extension of specified type for this channel.
       * \param[in] extension, channel extension smart pointer.
       *
       */
      QString extensionPath(const Core::StackExtensionSPtr extension) const
      { return extensionsPath() + extension->type() + "/"; }

      /** \brief Returns the path of the data files for a extension for this channel.
       *
       */
      QString extensionDataPath(const Core::StackExtensionSPtr extension, QString fileName) const
      { return extensionPath(extension) + QString("%1_%2").arg(uuid()).arg(fileName); }

    private:
      double  m_brightness; /** brightness.     [0-1]   */
      double  m_contrast;   /** contrast value. [0-1]   */
      double  m_hue;        /** hue value.      [0-359] */
      double  m_opacity;    /** opacity,        [0-1]   */
      double  m_saturation; /** saturation.     [0-1]   */

      mutable QString m_metadata; /** metadata information .*/
  };

}// namespace ESPINA

#endif // ESPINA_CHANNEL_H
