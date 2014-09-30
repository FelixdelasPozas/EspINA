/*
 * SliceContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef SLICECONTOURWIDGET_H_
#define SLICECONTOURWIDGET_H_


// ESPINA
#include "vtkPlaneContourWidget.h"
#include "ContourWidget.h"

#include <Core/EspinaTypes.h>
#include <GUI/Selectors/BrushSelector.h>

class vtkPolyData;

namespace ESPINA
{
class EspinaGUI_EXPORT SliceContourWidget
: public EspinaWidget
{
  public:
    explicit SliceContourWidget(vtkPlaneContourWidget *widget);
    virtual ~SliceContourWidget();

    virtual void setSlice(Nm pos, Plane plane);
    virtual void SetEnabled(int);
    QPair<BrushSelector::BrushMode, vtkPolyData *> getContour();

    void setMode(BrushSelector::BrushMode);

    void Initialize();
    void Initialize(ContourWidget::ContourData contour);
  private:

    bool                   m_initialized;
    Plane                  m_plane;
    Nm                     m_pos;
    vtkPlaneContourWidget *m_contourWidget;

    vtkPolyData             *m_storedContour;
    Nm                       m_storedContourPosition;
    BrushSelector::BrushMode m_storedContourMode;
};

}// namespace ESPINA

#endif /* SLICECONTOURWIDGET_H_ */
