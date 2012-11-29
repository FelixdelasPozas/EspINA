/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#ifndef SEGMHAIMPORTERFILTER_H
#define SEGMHAIMPORTERNFILTER_H

#include <Core/Model/Filter.h>

// Qt
#include <QColor>

class Segmentation;
class Channel;
class TaxonomyElement;
class Taxonomy;

class SegmhaImporterFilter 
: public Filter
{
  struct SegmentationObject
  {
    SegmentationObject(const QString &line);

    unsigned int label;
    unsigned int taxonomyId;
    unsigned char selected;
  };

  struct TaxonomyObject
  {
    TaxonomyObject(const QString &line);

    QString name;
    unsigned int label;
    QColor color;
  };

public:
  static const QString TYPE;
  static const QString SUPPORTED_FILES;

  static const ArgumentId FILE;
  static const ArgumentId BLOCKS;
  static const ArgumentId SPACING; //Some segmha files have wrong
                                   // spacing, we need to keep real one

  class Parameters
  {
  public:
    explicit Parameters(Arguments &args) : m_args(args) {}

    void setBlocks(QStringList blockList)
    {
      m_args[BLOCKS] = blockList.join(",");
    }
    QStringList blocks() const
    {
      return m_args[BLOCKS].split(",");
    }

    void setSpacing(itkVolumeType::SpacingType spacing)
    {
      m_args[SPACING] = QString("%1,%2,%3")
      .arg(spacing[0])
      .arg(spacing[1])
      .arg(spacing[2]);
    }
    itkVolumeType::SpacingType spacing()
    {
      itkVolumeType::SpacingType res;
      QStringList values = m_args[SPACING].split(",");

      for(int i=0; i<3; i++)
        res[i] = values[i].toDouble();

      return res;
    }
  private:
    Arguments &m_args;
  };

public:
  explicit SegmhaImporterFilter(NamedInputs inputs,
                                Arguments args);
  virtual ~SegmhaImporterFilter();

  // Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;
  virtual QString serialize() const;

  // Implements Filter Interface
  virtual bool needUpdate() const;

  /// Return full taxonomy contained in segmha's meta-data
  Taxonomy *taxonomy() {return m_taxonomy;}
  /// Return the taxonomy associated with the i-th output
  TaxonomyElement *taxonomy(OutputId i);
  /// Return Counting Region Definition
  void countingRegion(double inclusive[3], double exclusive[3])
  {
    memcpy(inclusive, m_inclusive, 3*sizeof(Nm));
    memcpy(exclusive, m_exclusive, 3*sizeof(Nm));
  }

  void initSegmentation(Segmentation *seg, OutputId i);

protected:
  virtual void run();

private:
  Parameters m_param;

  QList<TaxonomyElement *> m_taxonomies;
  QList<int>               m_labels;
  Taxonomy                *m_taxonomy;
  Nm                       m_inclusive[3], m_exclusive[3];
};


#endif // SEEDGROWSEGMENTATIONFILTER_H
