/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_SAMPLE_ADAPTER_H
#define ESPINA_SAMPLE_ADAPTER_H

#include "GUI/Model/NeuroItemAdapter.h"
#include <Core/Utils/NmVector3.h>
#include <Core/Utils/Bounds.h>

namespace EspINA
{
  class SampleAdapter;
  using SampleAdapterPtr   = SampleAdapter *;
  using SampleAdapterList  = QList<SampleAdapterPtr>;
  using SampleAdapterSPtr  = std::shared_ptr<SampleAdapter>;
  using SampleAdapterSList = QList<SampleAdapterSPtr>;

  /** \brief Sample 
   * 
   */
  class EspinaGUI_EXPORT SampleAdapter
  : public NeuroItemAdapter
  {
  public:
    virtual ~SampleAdapter();

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    virtual ItemAdapter::Type type() const
    { return Type::SAMPLE; }

    void setName(const QString& name);

    QString name() const;

    void setPosition(const NmVector3& point);

    NmVector3 position() const;

    /** \brief Set the spatial bounds in nm of the Sample in the Analysis frame reference
     */
    void setBounds(const Bounds& bounds);

    /** \brief Return the spatial bounds in nm of the Sample in the Analysis frame reference
     */
    Bounds bounds() const;

  private:
    explicit SampleAdapter(SampleSPtr sample);

  private:
    SampleSPtr m_sample;

    friend class ModelFactory;
    friend class ModelAdapter;
    friend class QueryAdapter;

    friend bool operator==(SampleAdapterSPtr lhs, SampleSPtr        rhs);
    friend bool operator==(SampleSPtr        lhs, SampleAdapterSPtr rhs);
  };

  bool operator==(SampleAdapterSPtr lhs, SampleSPtr        rhs);
  bool operator==(SampleSPtr        lhs, SampleAdapterSPtr rhs);

  bool operator!=(SampleAdapterSPtr lhs, SampleSPtr        rhs);
  bool operator!=(SampleSPtr        lhs, SampleAdapterSPtr rhs);

  SampleAdapterPtr samplePtr(ItemAdapterPtr item);

}// namespace EspINA

#endif // ESPINA_SAMPLE_ADAPTER_H
