/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SAS_INFORMATION_PROXY_H_
#define SAS_INFORMATION_PROXY_H_

// EspINA
#include <GUI/Model/Proxies/InformationProxy.h>

namespace EspINA
{
  class SASInformationProxy
  : public InformationProxy
  {
      class SASInformationFetcher;

    public:
      /* \brief SASInformationProxy class constructor.
       *
       */
      explicit SASInformationProxy(ModelAdapterSPtr model, SegmentationExtension::InfoTagList sasTags, SchedulerSPtr scheduler)
      : InformationProxy{scheduler}
      , m_model         {model}
      , m_sasTags       {sasTags}
      {};

      /* \brief SASInformationProxy class virtual destructor.
       *
       */
      virtual ~SASInformationProxy()
      {};

      /* \brief Implements QAbstractProxyModel::data()
       *
       */
      virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

    protected:
      ModelAdapterSPtr m_model;
      SegmentationExtension::InfoTagList m_sasTags;

  };

} // namespace EspINA

#endif // SAS_INFORMATION_PROXY_H_
