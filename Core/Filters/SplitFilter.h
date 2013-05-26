/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
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
 */


#ifndef SPLITFILTER_H
#define SPLITFILTER_H

#include <Core/Model/Filter.h>
#include <Core/OutputRepresentations/RawVolume.h>
#include "BasicSegmentationFilter.h"

#include <vtkSmartPointer.h>

class vtkImageStencilData;

namespace EspINA
{
  /// Split Segmentation into two components according to
  /// given stencil
  class SplitFilter
  : public BasicSegmentationFilter
  {
  public:
    static const QString INPUTLINK;

  public:
    explicit SplitFilter(NamedInputs inputs,
                         Arguments   args,
                         FilterType  type);
    virtual ~SplitFilter();

    void setStencil(vtkSmartPointer<vtkImageStencilData> stencil)
    {
      m_stencil = stencil;
      m_ignoreCurrentOutputs = true;
    }

    /// Try to locate an snapshot of the filter in tmpDir
    /// Returns true if all volume snapshot can be recovered
    /// and false otherwise
    virtual bool fetchCacheStencil();

    /// QMap<file name, file byte array> of filter's data to save to seg file
    virtual bool dumpSnapshot(QList<QPair<QString, QByteArray> > &fileList);

  protected:
    virtual bool ignoreCurrentOutputs() const
    { return m_ignoreCurrentOutputs; }

    virtual bool needUpdate(FilterOutputId oId) const;

    virtual void run();
    virtual void run(FilterOutputId oId);

  private:
    vtkSmartPointer<vtkImageStencilData> m_stencil;

    bool m_ignoreCurrentOutputs;
    RawSegmentationVolumeSPtr m_volumes[2];
  };

  typedef boost::shared_ptr<SplitFilter> SplitFilterSPtr;

} // namespace EspINA

#endif // SPLITFILTER_H
