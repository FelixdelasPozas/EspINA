/*
 * ContourSource.h
 *
 *  Created on: Sep 27, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef CONTOURSOURCE_H_
#define CONTOURSOURCE_H_

#include <model/Filter.h>
#include <vtkPolyData.h>
#include <QVector3D>

class ContourSource
: public Filter
{
public:
  class ContourSourceInspector;

  static const QString TYPE;

  static const ModelItem::ArgumentId SPACING;

  class Parameters
  {
  public:
    explicit Parameters(Arguments &args)
    : m_args(args)
    {
      QStringList values = m_args[SPACING].split(",", QString::SkipEmptyParts);
      if (values.size() == 3)
      {
        for(int i=0; i<3; i++)
          m_spacing[i] = values[i].toDouble();
      }
    }

    void setSpacing(double value[3])
    {
      for(int i=0; i<3; i++)
        m_spacing[i] = value[i];
      m_args[SPACING] = QString("%1,%2,%3")
                       .arg(value[0])
                       .arg(value[1])
                       .arg(value[2]);
    }
    EspinaVolume::SpacingType spacing() const
    {
      return m_spacing;
    }
  private:
    Arguments &m_args;
    EspinaVolume::SpacingType m_spacing;
  };

public:
  explicit ContourSource(NamedInputs inputs,
                          Arguments args);
  virtual ~ContourSource();

  virtual void draw(OutputId oId,
                    vtkPolyData *contour,
                    Nm slice, PlaneType plane,
                    EspinaVolume::PixelType value = SEG_VOXEL_VALUE);

  /// Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

  /// Implements Filter Interface
  virtual bool needUpdate() const;

  // to emit modified(this) after the last contour has been drawn to update views
  virtual void signalAsModified();

  virtual QWidget* createFilterInspector(QUndoStack* undoStack, ViewManager* vm);


protected:
  virtual void run(){}


private:
  friend class ContourSourceInspector;

  // helper method to rotate the vtkPolyData to the axial plane. actually
  // contours are stored this way in the filter
  vtkPolyData* TransformContour(PlaneType, vtkPolyData*);

  Parameters m_param;
  EspinaVolume::SpacingType   m_spacing;
  QMap< PlaneType, QMap<Nm, vtkPolyData*> > m_contourMap;
  bool ImageInitializedByFilter;
};


#endif /* CONTOURSOURCE_H_ */
