#ifndef _EM_SEGMENTATION_H_
#define _EM_SEGMENTATION_H_

#include <QObject>

//Forward declarations
class pqPipelineSource;

/// This class manages the segmentation process of EM image stacks
class EMSegmentation : public QObject
{
	Q_OBJECT
public:
	EMSegmentation();
	~EMSegmentation(){}

	/// Load a segmentation file or creates an empty one if a single
	/// stack is given
	void loadSegmentation();

	/// Saves the current segmentation in file
	void saveSegmentation();

	/// Appends a new filter to the preprocessing pipeline
	void appendPreprocessingFilter(pqPipelineSource *filter);

	/// Deletes all preprocessing filtering
	void clearPreprocessingPipeline();

public slots:
	/// Adds segmentation to the current image's segmentation
	void addSegmentation(pqPipelineSource *segmentation){}

private:
	pqPipelineSource *m_stack, *m_workingStack;
	QList<pqPipelineSource *> *m_preprocessingPipeline;
};

#endif// _EM_SEGMENTATION_H_
