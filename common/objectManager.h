#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <QObject>

//Forward declarations
class pqPipelineSource;

/// This class manages the segmentation process of EM image stacks
class ObjectManager : public QObject
{
	Q_OBJECT
public:
	ObjectManager();
	~ObjectManager(){}

	pqPipelineSource *stack(){return m_stack;}

	void setStack(pqPipelineSource *stack);

	/// Returns a pointer to the stack set by the user
	//  Note: Consider visualization pipeline?
	pqPipelineSource *visualizationStack();

	/// Returns a pointer to the working stack. This stack can differ from the one set by the user due to preprocessing pipeline
	pqPipelineSource *workingStack();

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
	void addSegmentation(pqPipelineSource *segmentation);

signals:
	void segmentationAdded(pqPipelineSource *);

private:
	pqPipelineSource *m_stack, *m_workingStack;
	QList<pqPipelineSource *> *m_preprocessingPipeline;
	QList<pqPipelineSource *> *m_segmentations;
};

#endif// OBJECT_MANAGER_H
