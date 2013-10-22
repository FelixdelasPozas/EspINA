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

#include <Core/Analysis/Extensions/ChannelExtension.h>
#include <Core/Utils/Bounds.h>

namespace EspINA
{

  class EspinaGUI_EXPORT ChannelAdapter
  : public ViewItemAdapter
  {
    virtual ~ChannelAdapter();

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    virtual Type type() const
    { return Type::CHANNEL; }

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

  protected:
    virtual AnalysisItemSPtr item() const
    { return m_channel; }

  private:
    explicit ChannelAdapter(ChannelSPtr channel);

  private:
    ChannelSPtr m_channel;

    friend class ModelFactory;
    friend class ModelAdapter;
  };

  FilterAdapterSPtr sgs = factory.createF(SGS, channel);
  ChannelItemAdapterSPtr channel = factory.createC(sgs, 0);

  model.add(channel);

//   class EspinaGUI_EXPORT Channel2
//   : public ItemAdapter
//   {
//   public:
//     // Argument Ids
//     static const ArgumentId ID;
//     static const ArgumentId HUE;
//     static const ArgumentId OPACITY;
//     static const ArgumentId SATURATION;
//     static const ArgumentId CONTRAST;
//     static const ArgumentId BRIGHTNESS;
//     static const ArgumentId VOLUME;
// 
//     static const QString LINK;
//     static const QString STAIN_LINK;
//     static const QString VOLUME_LINK;
// 
//     static const int AUTOMATIC_OPACITY = -1;
// 
//     // Extended Information and representation tags
//     static const QString NAME;
//     static const QString VOLUMETRIC;
// 
//     class CArguments
//     : public ModelItem::Arguments
//     {
//     public:
//       explicit CArguments() : m_outputId(0) {}
//       explicit CArguments(const Arguments &args)
//       : Arguments(args), m_outputId(0) {}
// 
//       /// Channel's hue. Value in range (0,1) U (-1), latter meaning not stained
//       void setHue(double hue)
//       {
//         (*this)[HUE] = QString::number(hue);
//       }
// 
//       double hue() const
//       {
//         return (*this)[HUE].toFloat();
//       }
// 
//       /// Channel's opacity. Value in range (0,1) U (-1), latter meaning automatically managed
//       void setOpacity(double opacity)
//       {
//         (*this)[OPACITY] = QString::number(opacity);
//       }
// 
//       double opacity() const
//       {
//         return (*this)[OPACITY].toFloat();
//       }
// 
//       /// Channel's saturation. Value in range (0,1).
//       void setSaturation(double saturation)
//       {
//         (*this)[SATURATION] = QString::number(saturation);
//       }
// 
//       double saturation() const
//       {
//         return (*this)[SATURATION].toFloat();
//       }
// 
//       /// Channel's contrast. Value in range (0,2).
//       void setContrast(double contrast)
//       {
//         (*this)[CONTRAST] = QString::number(contrast);
//       }
// 
//       double contrast() const
//       {
//         return (*this)[CONTRAST].toFloat();
//       }
// 
//       /// Channel's brightness. Value in range (-1,1).
//       void setBrightness(double brightness)
//       {
//         (*this)[BRIGHTNESS] = QString::number(brightness);
//       }
// 
//       double brightness() const
//       {
//         return (*this)[BRIGHTNESS].toFloat();
//       }
// 
//       void setOutputId(FilterOutputId oId)
//       {
//         (*this)[VOLUME] = QString("%1_%2")
//         .arg(VOLUME_LINK)
//         .arg(oId);
//         m_outputId = oId;
//       }
// 
//       FilterOutputId outputId() const
//       {
//         return m_outputId;
//       }
// 
//     private:
//       FilterOutputId m_outputId;
//     };
// 
//   public:
//     virtual ~Channel();
// 
//     void setVisible(bool visible) {m_visible = visible;}
//     bool isVisible() const {return m_visible;}
// 
//     void setHue(double hue);
//     double hue() const;
// 
//     void setOpacity(double opacity);
//     double opacity() const;
// 
//     void setSaturation(double saturation);
//     double saturation() const;
// 
//     void setContrast(double contrast);
//     double contrast() const;
// 
//     void setBrightness(double brightness);
//     double brightness() const;
// 
//     /// Model Item Interface
//     virtual QVariant data(int role=Qt::DisplayRole) const;
//     virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);
//     virtual ModelItemType type() const {return CHANNEL;}
//     virtual QString  serialize() const;
// 
//     virtual void initialize(const Arguments &args = Arguments());
// 
//     virtual void initializeExtensions();
//     virtual void invalidateExtensions();
// 
//     /// Get the sample which channel belongs to
//     SampleSPtr sample();
// 
//     /// Pickable Item Interface
//     virtual const FilterSPtr filter() const;
//     virtual FilterSPtr filter() { return PickableItem::filter(); }
//     virtual const FilterOutputId outputId() const;
// 
//     ChannelVolumeSPtr volume();
//     const ChannelVolumeSPtr volume() const;
// 
//     void setPosition(Nm pos[3]);
//     void position(Nm pos[3]);
// 
//     /// Add a new extension to the segmentation
//     /// Extesion won't be available until requirements are satisfied
//     void addExtension(Channel::ExtensionPtr extension);
//     /// Delete extension from channel
//     void deleteExtension(Channel::ExtensionPtr extension);
//     Channel::ExtensionPtr extension(ModelItem::ExtId extensionId);
// 
//   private:
//     explicit Channel(FilterSPtr filter, FilterOutputId oId);
//     friend class EspinaFactory;
//   private:
//     bool   m_visible;
//     Nm m_pos[3];
// 
//     mutable CArguments m_args;
// 
//     FilterSPtr        m_filter;
//     ExtensionProvider m_extensions;
//   };
// 
//   ChannelPtr  EspinaCore_EXPORT channelPtr(ModelItemPtr      item);
//   ChannelPtr  EspinaCore_EXPORT channelPtr(PickableItemPtr   item);
//   ChannelSPtr EspinaCore_EXPORT channelPtr(ModelItemSPtr    &item);
//   ChannelSPtr EspinaCore_EXPORT channelPtr(PickableItemSPtr &item);

}// namespace EspINA

#endif // CHANNEL_H
