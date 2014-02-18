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

#ifndef ESPINA_CHANNEL_ADAPTER_H
#define ESPINA_CHANNEL_ADAPTER_H

#include "GUI/Model/ViewItemAdapter.h"

#include <Core/Analysis/Extension.h>

namespace EspINA
{
  class ChannelAdapter;
  using ChannelAdapterPtr   = ChannelAdapter *;
  using ChannelAdapterList  = QList<ChannelAdapterPtr>;
  using ChannelAdapterSPtr  = std::shared_ptr<ChannelAdapter>;
  using ChannelAdapterSList = QList<ChannelAdapterSPtr>;

  class EspinaGUI_EXPORT ChannelAdapter
  : public ViewItemAdapter
  {
  public:
    virtual ~ChannelAdapter();

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    virtual ItemAdapter::Type type() const
    { return Type::CHANNEL; }

    virtual InputSPtr asInput() const;

    void setPosition(const NmVector3& point);

    NmVector3 position() const;

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
    double hue() const;

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
    double opacity() const;

    /** \brief Set channel's saturation
     *
     *  Saturation value belongs to [0,1].
     */
    void setSaturation(double saturation);

    /** \brief Channel's saturation
     *
     *  Saturation value belongs to [0,1]
     */
    double saturation() const;

    /** \brief Set channel's contrast
     *
     *  Contrast value belongs to [0,2]
     */
    void setContrast(double contrast);

    /** \brief Channel's contrast
     *
     *  Contrast value belongs to [0,2]
     */
    double contrast() const;

    /** \brief Set channel's brightness
     *
     *  Brightness value belongs to [-1,1]
     */
    void setBrightness(double brightness);

    /** \brief Channel's brightness
     *
     *  Brightness value belongs to [-1,1]
     */
    double brightness() const;

    Bounds bounds() const;

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
    explicit ChannelAdapter(FilterAdapterSPtr filter, ChannelSPtr channel);

  private:
    ChannelSPtr m_channel;

    friend class ModelFactory;
    friend class ModelAdapter;
    friend class QueryAdapter;

    friend bool operator==(ChannelAdapterSPtr lhs, ChannelSPtr        rhs);
    friend bool operator==(ChannelSPtr        lhs, ChannelAdapterSPtr rhs);
  };

  bool operator==(ChannelAdapterSPtr lhs, ChannelSPtr        rhs);
  bool operator==(ChannelSPtr        lhs, ChannelAdapterSPtr rhs);

  bool operator!=(ChannelAdapterSPtr lhs, ChannelSPtr        rhs);
  bool operator!=(ChannelSPtr        lhs, ChannelAdapterSPtr rhs);

  ChannelAdapterPtr EspinaGUI_EXPORT channelPtr(ItemAdapterPtr item);
}// namespace EspINA

#endif // CHANNEL_H
