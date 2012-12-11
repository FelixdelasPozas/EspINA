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

#ifndef RENDERER
#define RENDERER

#include <QString>
#include <QIcon>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

class ModelItem;

/// Base class which define the API to render and manage
/// item visibily in Espina Views (currently only supported
/// for VolumeView class)
class Renderer
: public QObject
{
  Q_OBJECT
public:
  virtual ~Renderer(){}

  /// Following methods are used by view settings' panel and the
  /// view itself to create the corresponding UI to manage rendering
  virtual const QString name() const {return QString();}
  virtual const QString tooltip() const {return QString();}
  virtual const QIcon icon() const {return QIcon();}

  virtual void setVtkRenderer(vtkSmartPointer<vtkRenderer> renderer) {m_renderer = renderer;}

  // Return whether the item was rendered or not
  virtual bool addItem(ModelItem *item) {return false;}
  virtual bool updateItem(ModelItem *item) {return false;}
  virtual bool removeItem(ModelItem *item) {return false;}

  // Hide/Show all items rendered by the Renderer
  virtual void hide() {};
  virtual void show() {};

  // Remove all items rendered by the Renderer
  virtual void clean() {}

  virtual Renderer *clone() = 0;

  // get number of vtkActors added to vtkRendered from this Renderer
  virtual unsigned int getNumberOfvtkActors(void) { return 0; }

  virtual bool isHidden() { return !m_enable; }

  // true if this renderer renders segmentations only
  virtual bool isASegmentationRenderer() { return false; };
public slots:
  virtual void setEnable(bool value)
  {
    if (value)
      show();
    else
      hide();
    // the subclass will alter the m_enable value
  }

signals:
  void renderRequested();

protected:
  explicit Renderer(QObject* parent = 0)
  : m_enable(false)
  , m_renderer(NULL) {}
protected:
  bool m_enable;
  vtkSmartPointer<vtkRenderer> m_renderer;
};

#endif // RENDERER

