/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerSNSnavExtensionModuleWidget_h
#define __qSlicerSNSnavExtensionModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerSNSnavExtensionModuleExport.h"

class qSlicerSNSnavExtensionModuleWidgetPrivate;
class vtkMRMLNode;

//	vtk includes
#include <vtkSmartPointer.h>
#include <vtkWorldPointPicker.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkMRMLAnnotationFiducialNode.h>
#include <vtkMRMLAnnotationHierarchyNode.h>
#include <vtkPoints.h>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkMatrix4x4.h>

//	igstk includes
#include "igstkTrackerInitializer.h"
#include "igstkTransformObserver.h"
#include "igstkPivotCalibration.h"

#include "AxialView.h"

#include <QtGui>

#include "ui_qSlicerSNSnavExtensionModule.h"

using namespace igstk;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_SNSNAVEXTENSION_EXPORT qSlicerSNSnavExtensionModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
	Ui::qSlicerSNSnavExtensionModule ui;

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerSNSnavExtensionModuleWidget(QWidget *parent=0);
  virtual ~qSlicerSNSnavExtensionModuleWidget();

  vtkSmartPointer<vtkMRMLAnnotationFiducialNode> m_fnode1,m_fnode2,m_fnode3,m_fnode4;
  vtkSmartPointer<vtkWorldPointPicker> m_Picker;
  vtkSmartPointer<vtkPoints> m_Points;

  int m_nCount;

  CAxialView* m_pAxialView;

  QTimer *m_refreshTimer;

  QString m_ProbeXml;
  QString m_ToolXml;

  igstk::TrackerInitializer         * m_TrackerInitializer;
  igstk::TrackerConfiguration       *m_TrackerConfiguration;

  typedef igstk::TransformObserver ObserverType;

  vtkSmartPointer<vtkMatrix4x4> m_ProbeMatrix;
  vtkSmartPointer<vtkMatrix4x4> m_RefMatrix;
  vtkSmartPointer<vtkMatrix4x4> m_CalibrateMatrix;
  vtkSmartPointer<vtkMatrix4x4> m_ToolMatrix;

  double m_fProbeTipOffset[3],m_fToolTipOffset[3],m_fToolAxis[3],m_fToolAxisQuaternion[4];

  PivotCalibration::Pointer m_pivotCalibration;

  //pivot calibration events (start, progress, end) 
  //observer
	typedef itk::MemberCommand<qSlicerSNSnavExtensionModuleWidget>   
	CalibrationObserverType;
	void OnCalibrationEvent( itk::Object *caller, 
						   const itk::EventObject & event );
	CalibrationObserverType::Pointer m_CalibrationObserver;

	//pivot calibration initialization observer
	typedef itk::MemberCommand<qSlicerSNSnavExtensionModuleWidget>   
	InitializationObserverType;
	void OnInitializationEvent( itk::Object *caller, 
							  const itk::EventObject & event );
	InitializationObserverType::Pointer m_InitializationObserver;

	//pivot calibration get transform observer
  igstkObserverMacro( TransformTo, 
                      igstk::CoordinateSystemTransformToEvent, 
                      igstk::CoordinateSystemTransformToResult )
  TransformToObserver::Pointer m_TransformToObserver;

  //pivot calibration get RMSE observer
  igstkObserverMacro( RMSE, 
                      igstk::DoubleTypeEvent, 
                      igstk::EventHelperType::DoubleType )
  RMSEObserver::Pointer m_RMSEObserver;

signals:
  void ProgressUpdate(int nProgress);

public slots:

	  void OnPickPoint();
	  void OnSurfacePick();
	  void OnSurfaceRegistration();
	  void OnPointShow();
	  void OnAllPointsDelete();
	  void OnPrePointDelete();
	  void OnStartTracking();
	  void OnStopTracking();
	  void timer_refresh();
	  void OnCalibrate();
	  void OnObtainAngle();
	  void advanceProgressBar(int nProgress);
	  void OnCreateProbeXml();
	  void OnCreateToolXml();
	  void Euler2Quaternion();
	  void OnSTART_OPENIGT();

protected:
  QScopedPointer<qSlicerSNSnavExtensionModuleWidgetPrivate> d_ptr;

  vtkRenderWindowInteractor* iren;
  vtkRenderWindow* renderWindow;
  vtkRenderer* renderer;

  QList<vtkSmartPointer<vtkWorldPointPicker> > pointList;

  virtual void setup();

protected slots:

  //void DrawPoint();

private:
  Q_DECLARE_PRIVATE(qSlicerSNSnavExtensionModuleWidget);
  Q_DISABLE_COPY(qSlicerSNSnavExtensionModuleWidget);
};

#endif
