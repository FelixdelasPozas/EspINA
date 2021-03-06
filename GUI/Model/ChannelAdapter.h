/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CHANNEL_ADAPTER_H
#define ESPINA_CHANNEL_ADAPTER_H

// ESPINA
#include "GUI/Model/ViewItemAdapter.h"
#include <Core/Analysis/Extensible.hxx>
#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  /** \class ChannelAdapter
   * \brief Adapts the Channel object for a Qt model.
   *
   */
  class EspinaGUI_EXPORT ChannelAdapter
  : public ViewItemAdapter
  {
    public:
      using ReadLockExtensions  = Core::ReadLockExtensions<Core::StackExtension, Channel>;
      using WriteLockExtensions = Core::WriteLockExtensions<Core::StackExtension, Channel>;

    public:
      /** \brief ChannelAdapter class destructor.
       *
       */
      virtual ~ChannelAdapter();

      virtual QVariant data(int role = Qt::DisplayRole) const;

      virtual bool setData(const QVariant& value, int role);

      virtual ItemAdapter::Type type() const
      { return Type::CHANNEL; }

      virtual InputSPtr asInput() const;

      /** \brief Sets the position of the channel (origin).
       * \param[in] point origin point.
       *
       */
      void setPosition(const NmVector3& point);

      /** \brief Returns the position (origin) of the channel.
       *
       */
      NmVector3 position() const;

      /** \brief Set channel's hue value.
       * \param[in] hue hue value.
       *
       *  Hue value belongs to [0,1] U -1.\n
       *  Not stained channels hue value is -1
       */
      void setHue(double hue);

      /** \brief Returns the channel's hue.
       *
       *  Hue value belongs to [0,1] U -1.\n
       *  Not stained channels hue value is -1
       */
      double hue() const;

      /** \brief Set channel's opacity.
       * \param[in] opacity opacity value.
       *
       * Opacity value belong to [0,1] U -1 \n
       * -1 value means opacity is automatically managed
       */
      void setOpacity(double opacity);

      /** \brief Returns the channel's opacity
       *
       * Opacity value belong to [0,1] U -1 \n
       * -1 value means opacity is automatically managed
       */
      double opacity() const;

      /** \brief Set channel's saturation.
       * \param[in] saturation saturation value.
       *
       *  Saturation value belongs to [0,1].
       */
      void setSaturation(double saturation);

      /** \brief Returns the channel's saturation.
       *
       *  Saturation value belongs to [0,1]
       */
      double saturation() const;

      /** \brief Set channel's contrast.
       * \param[in] contrast contrast value.
       *
       *  Contrast value belongs to [0,2]
       */
      void setContrast(double contrast);

      /** \brief Returns the channel's contrast.
       *
       *  Contrast value belongs to [0,2]
       */
      double contrast() const;

      /** \brief Set channel's brightness.
       * \param[in] brightness brightness value.
       *
       *  Brightness value belongs to [-1,1]
       */
      void setBrightness(double brightness);

      /** \brief Returns the channel's brightness.
       *
       *  Brightness value belongs to [-1,1]
       */
      double brightness() const;

      /** \brief Set channel's metadata.
       * \param[in] metadata channel's metadata as string.
       *
       */
      void setMetadata(const QString& metadata);

      /** \brief Returns the channel's metadata.
       *
       */
      QString metadata() const;

      /** \brief Returns the minimal bounds that contains the channel.
       *
       */
      Bounds bounds() const;

      /** \brief Returns the item extensions as read only.
       *
       */
      ReadLockExtensions readOnlyExtensions() const;

      /** \brief Returns the item extensions.
       *
       */
      WriteLockExtensions extensions();

    protected:
      virtual void changeOutputImplementation(InputSPtr input);

    private:
      /** \brief ChannelAdapter class private constructor.
       * \param[in] filter filter adapter smart pointer.
       * \param[in] channel smart pointer of the channel to adapt.
       *
       */
      explicit ChannelAdapter(ChannelSPtr channel);

    private:
      ChannelSPtr m_channel; /** adapted channel object. */

      friend class ModelFactory;
      friend class ModelAdapter;
      friend class QueryAdapter;

      /** \brief Equal operator between channel objects and channelAdapter objects.
       * \param[in] lhs ChannelAdapter smart pointer.
       * \param[in] rhs Channel smart pointer.
       *
       */
      friend bool operator==(ChannelAdapterSPtr lhs, ChannelSPtr        rhs);

      /** \brief Equal operator between channel objects and channelAdapter objects.
       * \param[in] lhs Channel smart pointer.
       * \param[in] rhs ChannelAdapter smart pointer.
       *
       */
      friend bool operator==(ChannelSPtr        lhs, ChannelAdapterSPtr rhs);
  };

  /** \brief Equality operator for a channel adapter smart pointer and a channel smart pointer.
   * \param[in] lhs channel adapter smart pointer.
   * \param[in] rhs channel smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator==(ChannelAdapterSPtr lhs, ChannelSPtr rhs);

  /** \brief Equality operator for a channel smart pointer and a channel adapter smart pointer.
   * \param[in] lhs channel smart pointer.
   * \param[in] rhs channel adapter smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator==(ChannelSPtr lhs, ChannelAdapterSPtr rhs);

  /** \brief Inequality operator for a channel adapter smart pointer and a channel smart pointer.
   * \param[in] lhs channel adapter smart pointer.
   * \param[in] rhs channel smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator!=(ChannelAdapterSPtr lhs, ChannelSPtr        rhs);

  /** \brief Inequality operator for a channel smart pointer and a channel adapter smart pointer.
   * \param[in] lhs channel smart pointer.
   * \param[in] rhs channel adapter smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator!=(ChannelSPtr        lhs, ChannelAdapterSPtr rhs);

  /** \brief Returns the stack pointer of the givem item, if it's a stack or nullptr otherwise.
   * \param[in] item item pointer to convert to stack pointer.
   *
   */
  ChannelAdapterPtr EspinaGUI_EXPORT channelPtr(ItemAdapterPtr item);

  /** \brief Returns the stack pointer to const of the givem item, if it's a stack or nullptr otherwise.
   * \param[in] item item pointer to const to convert to stack pointer to const.
   *
   */
  ConstChannelAdapterPtr EspinaGUI_EXPORT channelPtr(ConstItemAdapterPtr item);

  /** \brief Returns true if the given item is a channel item.
   * \param[in] item item adapter raw pointer.
   *
   */
  bool EspinaGUI_EXPORT isChannel(ItemAdapterPtr item);

  /** \brief Converts and returns the given stack smart pointer to a view item adapter smart pointer list.
   * \param[in] stack stack smart pointer.
   *
   *
   */
  ViewItemAdapterSList EspinaGUI_EXPORT toViewItemSList(ChannelAdapterSPtr stack);

  /** \brief Converts and returns the given stack smart pointer list to a view item adapter smart pointer list.
   * \param[in] stacks list of stack smart pointers.
   *
   *
   */
  ViewItemAdapterSList EspinaGUI_EXPORT toViewItemSList(ChannelAdapterSList stacks);

}// namespace ESPINA

#endif // CHANNEL_H
