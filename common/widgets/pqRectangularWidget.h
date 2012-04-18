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

#ifndef PQRECTANGULARWIDGET_H
#define PQRECTANGULARWIDGET_H

#include <pq3DWidget.h>

class vtkSMProxy;
class pqServer;

/// Provides UI for Rectangular Widget.
class PQCOMPONENTS_EXPORT pqRectangularWidget : public pq3DWidget
{
  Q_OBJECT
  typedef pq3DWidget Superclass;
public:
  pqRectangularWidget(vtkSMProxy* refProxy, vtkSMProxy* proxy, QWidget* p = 0);
  virtual ~pqRectangularWidget();

  /// Resets the bounds of the 3D widget to the reference proxy bounds.
  /// This typically calls PlaceWidget on the underlying 3D Widget 
  /// with reference proxy bounds.
  /// This should be explicitly called after the panel is created
  /// and the widget is initialized i.e. the reference proxy, controlled proxy
  /// and hints have been set.
  virtual void resetBounds(double bounds[6]);
  virtual void resetBounds()
    { this->Superclass::resetBounds(); }

  /// accept the changes.
  virtual void accept();

  /// reset the changes.
  virtual void reset();

  /// Overridden to update widget placement based on data bounds.
  virtual void select();

protected:
  /// Internal method to create the widget.
  void createWidget(pqServer*);

  /// Internal method to cleanup widget.
  void cleanupWidget();

  void updateControlledFilter();
  void updateWidgetMargins();

private slots:
  /// Called when the user changes widget visibility
  void onWidgetVisibilityChanged(bool visible);

private:
  pqRectangularWidget(const pqRectangularWidget&); // Not implemented.
  void operator=(const pqRectangularWidget&); // Not implemented.

  vtkSMProxy *InternalProxy;
};

#endif // PQRECTANGULARWIDGET_H


