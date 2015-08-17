/*
 
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <GUI/Model/Proxies/InformationProxy.h>

namespace ESPINA
{
  class SASInformationProxy
  : public InformationProxy
  {
    class SASInformationFetcher;

  public:
    /** \brief SASInformationProxy class constructor.
     *
     */
    explicit SASInformationProxy(ModelAdapterSPtr model, SegmentationExtension::InformationKeyList sasTags, SchedulerSPtr scheduler)
    : InformationProxy{scheduler}
    , m_model         {model}
    , m_sasTags       {sasTags}
    {};

    /** \brief SASInformationProxy class virtual destructor.
     *
     */
    virtual ~SASInformationProxy()
    {};

    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

  private:
    QVariant information(SegmentationAdapterPtr segmentation,
                         SegmentationAdapterPtr sas,
                         SegmentationExtension::InformationKey& key) const;

  protected:
    ModelAdapterSPtr m_model;
    SegmentationExtension::InformationKeyList m_sasTags;
  };

} // namespace ESPINA

#endif // SAS_INFORMATION_PROXY_H_
