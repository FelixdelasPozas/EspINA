#ifndef _RENDERER_H_
#define _RENDERER_H_

class pqRenderView;
class Segmentation;
class pqRenderView;

enum RenderType
{
	RENDERTYPE_FIRST = 0
	, MESH_RENDERER = 0
	, VOLUME_RENDERER = 1
	, RENDERTYPE_LAST = 1
};

class Renderer
{
public:
	Renderer(){}
	virtual ~Renderer() {};

	virtual RenderType type() = 0;
	
	virtual void hide(Segmentation *seg, pqRenderView *view) = 0;
	virtual void render(Segmentation *seg, pqRenderView *view) = 0;
	virtual void renderSelection(Segmentation *seg, pqRenderView *view) = 0;
	virtual void renderDiscarted(Segmentation *seg, pqRenderView *view) = 0;
};

class MeshRenderer : public Renderer
{
public:
	MeshRenderer() : Renderer(){}
	~MeshRenderer(){}
	
	static Renderer *renderer();

	RenderType type() {return MESH_RENDERER;}
	
	void hide(Segmentation *seg, pqRenderView *view);
	void render(Segmentation *seg, pqRenderView *view);
	void renderSelection(Segmentation *seg, pqRenderView *view);
	void renderDiscarted(Segmentation *seg, pqRenderView *view);

private:
	static MeshRenderer *m_singleton;
	
};

class VolumeRenderer : public Renderer
{
public:
	VolumeRenderer() : Renderer(){}
	~VolumeRenderer(){}
	
	static Renderer *renderer();

	RenderType type() {return VOLUME_RENDERER;}
	
	void hide(Segmentation *seg, pqRenderView *view);
	void render(Segmentation *seg, pqRenderView *view);
	void renderSelection(Segmentation *seg, pqRenderView *view);
	void renderDiscarted(Segmentation *seg, pqRenderView *view);

private:
	static VolumeRenderer *m_singleton;
};

#endif// _RENDERER_H_
