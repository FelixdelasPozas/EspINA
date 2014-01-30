/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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


#ifndef ESPINA_VISUALIZATION_STATE_H
#define ESPINA_VISUALIZATION_STATE_H

#include "EspinaGUI_Export.h"

#include <Core/EspinaTypes.h>
#include <Core/Analysis/Extension.h>

// ITK
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

namespace EspINA
{
  class EspinaGUI_EXPORT VisualizationState
  : public SegmentationExtension
  {
  public:
    static const Type TYPE;

  public:
    explicit VisualizationState();
    virtual ~VisualizationState();

    virtual Type type() const
    { return TYPE; }

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual void onSegmentationSet(SegmentationPtr seg);

    virtual bool validCategory(const QString& classificationName) const
    { return true; }

    virtual InfoTagList availableInformations() const;

    virtual QVariant information(const InfoTag &tag) const;

//     virtual void loadCache(QuaZipFile  &file,
//                            const QDir  &tmpDir,
//                            IEspinaModel *model);
// 
//     virtual bool saveCache(Snapshot &snapshot);


    void setState(const QString& representation, const QString& state);

    QString state(const QString& representation);

  private:
    QMap<QString, QString> m_state;
  };

  using VisualizationStateSPtr = std::shared_ptr<VisualizationState>;

}// namespace EspINA


#endif // ESPINA_VISUALIZATION_STATE_H
