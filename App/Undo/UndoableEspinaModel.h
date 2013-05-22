/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef UNDOABLEESPINAMODEL_H
#define UNDOABLEESPINAMODEL_H

#include <Core/Model/EspinaModel.h>

class QUndoStack;

namespace EspINA
{

  class UndoableEspinaModel
  : public IEspinaModel
  {
  public:
    explicit UndoableEspinaModel(EspinaModel *model, QUndoStack *undoStack)
    : m_model(model)
    , m_undoStack(undoStack) 
    {}

    virtual ~UndoableEspinaModel() {}

    virtual EspinaFactory *factory() const
    { return m_model->factory(); }

    virtual bool isTraceable() const
    { return m_model->isTraceable(); }

    virtual void setTraceable(bool traceable)
    { m_model->setTraceable(traceable); }

    //---------------------------------------------------------------------------
    /************************* Model Item API *******************************/
    //---------------------------------------------------------------------------
    // Returns the taxonomy used by the analyzer
    virtual void setTaxonomy(TaxonomySPtr tax);

    virtual const TaxonomySPtr taxonomy() const
    { return m_model->taxonomy(); }

    virtual void addTaxonomy(TaxonomySPtr taxonomy);

    virtual TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementPtr  parent, const QString &name);

    virtual TaxonomyElementSPtr createTaxonomyElement(TaxonomyElementSPtr parent, const QString &name);

    virtual void addTaxonomyElement   (TaxonomyElementSPtr parent, TaxonomyElementSPtr element);

    virtual void removeTaxonomyElement(TaxonomyElementSPtr parent, TaxonomyElementSPtr element);

    // Samples
    virtual void addSample(SampleSPtr    sample );

    virtual void addSample(SampleSList   samples);

    virtual void removeSample(SampleSPtr sample );

    virtual SampleSList samples() const
    { return m_model->samples(); }

    // Channels
    virtual void addChannel   (ChannelSPtr   channel  );

    virtual void addChannel   (ChannelSList  channels );

    virtual void removeChannel(ChannelSPtr   channel  );

    virtual ChannelSList channels() const
    { return m_model->channels(); }

    // Segmentations
    virtual void addSegmentation   (SegmentationSPtr   segmentation  );

    virtual void addSegmentation   (SegmentationSList  segmentations );

    virtual void removeSegmentation(SegmentationSPtr   segmentation  );

    virtual void removeSegmentation(SegmentationSList  segmentations );

    virtual SegmentationSList segmentations() const
    { return m_model->segmentations(); }

    // Filters
    virtual void addFilter   (FilterSPtr     filter  );

    virtual void addFilter   (FilterSList filters );

    virtual void removeFilter(FilterSPtr     filter  );

    virtual FilterSList filters() const
    { return m_model->filters(); }

    //---------------------------------------------------------------------------
    /************************* Relationships API *******************************/
    //---------------------------------------------------------------------------
    virtual void addRelation   (ModelItemSPtr   ancestor,
                                ModelItemSPtr   succesor,
                                const QString &relation);


    virtual void removeRelation(ModelItemSPtr   ancestor,
                                ModelItemSPtr   succesor,
                                const QString &relation);


    virtual ModelItemSList relatedItems(ModelItemPtr   item,
                                        RelationType   relType,
                                        const QString &relName = "")
    { return m_model->relatedItems(item, relType, relName); }


    virtual RelationList relations(ModelItemPtr   item,
                                   const QString &relName = "")
    { return m_model->relations(item, relName); }


    virtual RelationshipGraphPtr relationships()
    { return m_model->relationships(); }

    //---------------------------------------------------------------------------
    /************************** SmartPointer API *******************************/
    //---------------------------------------------------------------------------
    virtual ModelItemSPtr find(ModelItemPtr item)
    { return m_model->find(item); }

    virtual TaxonomyElementSPtr findTaxonomyElement(ModelItemPtr       item           )
    { return m_model->findTaxonomyElement(item); }

    virtual TaxonomyElementSPtr findTaxonomyElement(TaxonomyElementPtr taxonomyElement)
    { return m_model->findTaxonomyElement(taxonomyElement) ; }

    virtual SampleSPtr findSample(ModelItemPtr item  )
    { return m_model->findSample(item); }

    virtual SampleSPtr findSample(SamplePtr    sample)
    { return m_model->findSample(sample); }

    virtual ChannelSPtr findChannel(ModelItemPtr item   )
    { return m_model->findChannel(item); }

    virtual ChannelSPtr findChannel(ChannelPtr   channel)
    { return m_model->findChannel(channel); }

    virtual SegmentationSPtr findSegmentation(ModelItemPtr    item        )
    { return m_model->findSegmentation(item); }

    virtual SegmentationSPtr findSegmentation(SegmentationPtr segmentation)
    { return m_model->findSegmentation(segmentation); }

    virtual FilterSPtr findFilter(ModelItemPtr item  )
    { return m_model->findFilter(item); }

    virtual FilterSPtr findFilter(FilterPtr    filter)
    { return m_model->findFilter(filter); }

  private:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
  };

} // namespace EspINA

#endif // UNDOABLEESPINAMODEL_H
