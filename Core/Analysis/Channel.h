/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
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

#include "EspinaCore_Export.h"

#include "Core/Analysis/Data.h"
#include "Core/Analysis/AnalysisItem.h"
#include "Core/Analysis/Persistent.h"
#include "Core/Analysis/Extensions/Extensible.h"
#include "Core/Analysis/Extensions/ChannelExtension.h"

namespace EspINA
{
  class EspinaCore_EXPORT Channel
  : public AnalysisItem
  , public Persistent
  , public Extensible
  {
  public:
    static const RelationName LINK;
    static const RelationName STAIN_LINK;
    static const RelationName VOLUME_LINK;

    // Extended Information and representation tags
    static const QString NAME;
    static const QString VOLUMETRIC;

  public:
    virtual ~Channel();

    virtual void restoreState(const State& state);

    virtual std::ostream saveState() const;

    virtual void saveSnapshot(StorageSPtr storage) const;

    virtual void initializeExtensions();

    virtual void invalidateExtensions();

    void setPosition(Nm point[3]);

    void position(Nm point[3]);

    /** \brief Set channel's hue
     *
     *  Hue value belongs to [0,1] U -1.\n
     *  Not stained channels hue value is -1
     */
    void setHue(double hue);

    /** \brief Channel's hue
     *
     *  Hue value belongs to [0,1] U -1.\n
     *  Not stained channels hue value is -1
     */
    double hue() const
    { return m_hue; }

    /** \brief Set channel's opacity
     * 
     * Opacity value belong to [0,1] U -1 \n
     * -1 value means opacity is automatically managed
     */
    void setOpacity(double opacity);
    
    /** \brief Channel's opacity
     * 
     * Opacity value belong to [0,1] U -1 \n
     * -1 value means opacity is automatically managed
     */
    double opacity() const
    { return m_opacity; }

    /** \brief Set channel's saturation
     *
     *  Saturation value belongs to [0,1].
     */
    void setSaturation(double saturation);

    /** \brief Channel's saturation
     *
     *  Saturation value belongs to [0,1]
     */
    double saturation() const
    { return m_saturation; }

    /** \brief Set channel's contrast
     *
     *  Contrast value belongs to [0,2]
     */
    void setContrast(double contrast);

    /** \brief Channel's contrast
     *
     *  Contrast value belongs to [0,2]
     */
    double contrast() const
    { return m_contrast; }

    /** \brief Set channel's brightness
     *
     *  Brightness value belongs to [-1,1]
     */
    void setBrightness(double brightness);

    /** \brief Channel's brightness
     *
     *  Brightness value belongs to [-1,1]
     */
    double brightness() const
    { return m_brightness; }

    Bounds bounds() const;

    DataSPtr data(Data::Type type);
    const DataSPtr data(Data::Type type) const;

    /**
     *  Extesion won't be available until requirements are satisfied
     */
    void addExtension(ChannelExtensionSPtr extension);

    void deleteExtension(ChannelExtensionSPtr extension);

    /** \brief Check whether or not there is an extension with the given name
     *
     */
    bool hasExtension(const ChannelExtension::Type& type) const;

    /** \brief Return the extension with the especified name
     *
     *  Important: It the channel doesn't contain any extension with
     *  the requested name, but there exist an extension prototype registered
     *  in the factory, a new instance will be created and attached to the
     *  channel.
     *  If there is no extension with the given name registered in the factory
     *  a Undefined_Extension exception will be thrown
     */
    ChannelExtensionSPtr extension(const ChannelExtension::Type& type);

  private:
    explicit Channel(OutputSPtr output);
    friend class CoreFactory;

  private:
    static const int AUTOMATIC_OPACITY = -1;

    double m_brightness;
    double m_contrast;
    double m_hue;
    double m_opacity;
    double m_saturation;

    OutputSPtr            m_output;
    ChannelExtensionSList m_extensions;
  };

}// namespace EspINA

#endif // ESPINA_CHANNEL_H