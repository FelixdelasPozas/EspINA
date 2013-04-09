/*
 * ContourSource.h
 *
 *  Created on: Sep 27, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef CONTOURSOURCE_H_
#define CONTOURSOURCE_H_

#include <Core/Model/Filter.h>
#include <vtkPolyData.h>
#include <QVector3D>

namespace EspINA
{
  class ContourSource
  : public SegmentationFilter
  {
    public:
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
              for (int i = 0; i < 3; i++)
                m_spacing[i] = values[i].toDouble();
            }
          }

          void setSpacing(double value[3])
          {
            for (int i = 0; i < 3; i++)
              m_spacing[i] = value[i];
            m_args[SPACING] = QString("%1,%2,%3").arg(value[0]).arg(value[1]).arg(value[2]);
          }
          itkVolumeType::SpacingType spacing() const
          {
            return m_spacing;
          }
        private:
          Arguments &m_args;
          itkVolumeType::SpacingType m_spacing;
      };

    public:
      explicit ContourSource(NamedInputs inputs, Arguments args, FilterType type);
      virtual ~ContourSource() {};

      virtual void draw(OutputId oId, vtkPolyData *contour, Nm slice, PlaneType plane, itkVolumeType::PixelType value =
          SEG_VOXEL_VALUE, bool emitSignal = true);

    protected:
      virtual bool ignoreCurrentOutputs() const
      { return false; }

      virtual bool needUpdate(OutputId oId) const;

      virtual void run() {}

      virtual void run(OutputId oId) {}

    private:
      Parameters m_param;
  };

} // namespace EspINA

#endif /* CONTOURSOURCE_H_ */
