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


#ifndef ADAPTIVEBOUNDINGREGION_H
#define ADAPTIVEBOUNDINGREGION_H

#include "regions/BoundingRegion.h"

class Channel;

class AdaptiveBoundingRegion
: public BoundingRegion
{
public:
  static const QString ID;

  explicit AdaptiveBoundingRegion(CountingRegionChannelExtension *channelExt,
                                  Nm inclusion[3],
                                  Nm exclusion[3],
                                  ViewManager *vm);

  virtual ~AdaptiveBoundingRegion();

  // Implements QStandardItem interface
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual QString serialize() const;
  virtual QString regionType() const { return tr("Adaptive Region"); }

  // Implements EspinaWidget itnerface
  virtual vtkAbstractWidget *createWidget();
  virtual void deleteWidget(vtkAbstractWidget* widget);
  virtual SliceWidget *createSliceWidget(PlaneType plane);

  virtual bool processEvent(vtkRenderWindowInteractor* iren,
                            long unsigned int event);
  virtual void setEnabled(bool enable);

  virtual void updateBoundingRegionImplementation();

protected:
  double leftOffset()   const {return  m_inclusion[0];}
  double topOffset()    const {return  m_inclusion[1];}
  double upperOffset()  const {return  m_inclusion[2];}
  double rightOffset()  const {return -m_exclusion[0];}
  double bottomOffset() const {return -m_exclusion[1];}
  double lowerOffset()  const {return -m_exclusion[2];}
  void applyOffset(double &var, double offset)
  {var = floor(var + offset + 0.5);}

private:
  Channel *m_channel;
};

#endif // ADAPTIVEBOUNDINGREGION_H
