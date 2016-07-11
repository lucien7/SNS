
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkLandmarkTransform.h>

class SurfaceRegistration
{	
public:
	SurfaceRegistration(void);
public:
	~SurfaceRegistration(void);

public:
	vtkPoints *glyphPoints;
	vtkPolyData *glyphInputData;
	vtkSphereSource *glyphBalls;
	vtkGlyph3D *glyphPoints3D;
	vtkPolyDataMapper *glyphMapper;
	vtkActor *glyphActor;
	vtkMatrix4x4 *Matrix;
	vtkMatrix4x4 *m_RegisMatrix;

	int m_nTotalPolyPoint;

	void DrawSurfacePoint(double *fTipPoint,vtkRenderer *ren,vtkRenderWindow *renWin);
	void Registration(vtkPolyData *TargetPolyData,vtkMatrix4x4 *OriginalMatrix,vtkRenderWindow *renWin);
	void HidePoints(vtkRenderWindow *renWin);
	void ShowPoints(vtkRenderWindow *renWin);
	void DeleteAllPoints(vtkRenderer *ren,vtkRenderWindow *renWin);
	void DeletePrePoint(vtkRenderWindow *renWin);
};
