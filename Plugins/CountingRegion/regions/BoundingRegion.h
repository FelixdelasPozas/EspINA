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


#ifndef BOUNDINGREGION_H
#define BOUNDINGREGION_H

#include <QStandardItemModel>

#include "vtkBoundingRegionSliceWidget.h"
#include "vtkBoundingRegion3DWidget.h"
#include <GUI/vtkWidgets/EspinaWidget.h>
#include <GUI/vtkWidgets/EspinaInteractorAdapter.h>


#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

class CountingRegionChannelExtension;
class ViewManager;

/// Bounding Regions' base class
class BoundingRegion
: public QObject
, public QStandardItem
, public EspinaWidget
, public vtkCommand
{
  Q_OBJECT

protected:
  typedef EspinaInteractorAdapter<vtkBoundingRegionSliceWidget> BoundingRegion2DWidgetAdapter;
  typedef EspinaInteractorAdapter<vtkBoundingRegion3DWidget>    BoundingRegion3DWidgetAdapter;

  class BoundingRegionSliceWidget
  : public SliceWidget
  {
  public:
    explicit BoundingRegionSliceWidget(vtkBoundingRegionSliceWidget *widget)
    : SliceWidget(widget)
    , m_slicedWidget(widget)
    {}

    virtual void setSlice(Nm pos, PlaneType plane)
    {
      m_slicedWidget->SetSlice(pos);
      SliceWidget::setSlice(pos, plane);
    }
  private:
    vtkBoundingRegionSliceWidget *m_slicedWidget;
  };

public:
  const int INCLUSION_FACE;
  const int EXCLUSION_FACE;

  enum Role
  {
    DescriptionRole = Qt::UserRole + 1
  };
public:
  vtkTypeMacro(BoundingRegion, vtkCommand);
  explicit BoundingRegion(CountingRegionChannelExtension *channelExt,
                          Nm inclusion[3],
                          Nm exclusion[3],
                          ViewManager *vm);

  virtual ~BoundingRegion(){}

  void setMargins(Nm inclusion[3], Nm exclusion[3]);

  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual QString serialize() const = 0;
  virtual QString regionType() const = 0;

  /// Return total volume in pixels
  virtual double totalVolume() const
  { return m_totalVolume; }
  /// Return inclusion volume in pixels
  virtual double inclusionVolume() const
  { return m_inclusionVolume; }
  /// Return exclusion volume in pixels
  virtual double exclusionVolume() const
  { return totalVolume() - inclusionVolume(); }

  virtual vtkSmartPointer<vtkPolyData> region() const {return m_boundingRegion;}

  virtual void Execute(vtkObject* caller, long unsigned int eventId, void* callData);

  Nm left()  const {return m_inclusion[0];}
  Nm top()   const {return m_inclusion[1];}
  Nm upper() const {return m_inclusion[2];}
  Nm right() const {return m_exclusion[0];}
  Nm bottom()const {return m_exclusion[1];}
  Nm lower() const {return m_exclusion[2];}


signals:
  void modified(BoundingRegion *);

protected:
  void updateBoundingRegion();
  virtual void updateBoundingRegionImplementation() = 0;

protected:
  ViewManager *m_viewManager;
  CountingRegionChannelExtension *m_channelExt;

  vtkSmartPointer<vtkPolyData> m_boundingRegion;
  vtkSmartPointer<vtkPolyData> m_representation;

  Nm m_inclusion[3];
  Nm m_exclusion[3];
  Nm m_totalVolume, m_inclusionVolume;

  QList<BoundingRegion2DWidgetAdapter *> m_widgets2D;
  QList<BoundingRegion3DWidgetAdapter *> m_widgets3D;
};

#endif // BOUNDINGREGION_H
