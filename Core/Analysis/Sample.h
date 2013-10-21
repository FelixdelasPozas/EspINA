/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    Sample.h
// Purpose: Model a physical sample.
//          It provides channels and segmentations with a link to the
//          physical world
//----------------------------------------------------------------------------
#ifndef ESPINA_SAMPLE_H
#define ESPINA_SAMPLE_H

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"

#include "Core/Utils/Bounds.h"
#include "Core/Analysis/Persistent.h"
#include "AnalysisItem.h"

namespace EspINA
{

  typedef QList<SampleSPtr>      SampleSList;

  /** \brief Sample 
   * 
   */
  class EspinaCore_EXPORT Sample
  : public AnalysisItem
  , public Persistent
  {
  public:
    virtual ~Sample();

    virtual void restoreState(const State& state);

    virtual std::ostream saveState() const;

    virtual void saveSnapshot(StorageSPtr storage) const;

    virtual void unload();

    void setName(const QString& name)
    { m_name = name; }

    QString name() const
    { return m_name; }

    void setPosition(Nm point[3]);
    //Nm &&position() const;
    void position(Nm point[3]) const;

    /** \brief Set the spatial bounds in nm of the Sample in the Analysis frame reference
     */
    void setBounds(const Bounds& bounds)
    { m_bounds = bounds; }
    
    /** \brief Return the spatial bounds in nm of the Sample in the Analysis frame reference
     */
    Bounds bounds() const
    { return m_bounds; }
    
  private:
    explicit Sample(const QString& name);
    friend class CoreFactory;

  private:
    QString m_name;
    Bounds  m_bounds;
  };
}// namespace EspINA

#endif // ESPINA_SAMPLE_H
