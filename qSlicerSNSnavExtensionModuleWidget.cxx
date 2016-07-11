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

// Qt includes
#include <QDebug>
#include <QList>
#include <QMessageBox>
#include <QString>

// vtk includes
#include <vtkPointPicker.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// SlicerQt includes
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

#include <vtkMRMLScene.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLAnnotationHierarchyNode.h>
#include <vtkMRMLAnnotationFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>

#include <vtkSlicerSNSnavExtensionLogic.h>
#include "ui_qSlicerSNSnavExtensionModule.h"  
#include "qSlicerSNSnavExtensionModuleWidget.h"

#include   <shellapi.h> 

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSNSnavExtensionModuleWidgetPrivate: public Ui_qSlicerSNSnavExtensionModule
{
public:
  qSlicerSNSnavExtensionModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSNSnavExtensionModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerSNSnavExtensionModuleWidgetPrivate::qSlicerSNSnavExtensionModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSNSnavExtensionModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSNSnavExtensionModuleWidget::qSlicerSNSnavExtensionModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerSNSnavExtensionModuleWidgetPrivate )
{
	qSlicerApplication *app = qSlicerApplication::application();
    qSlicerLayoutManager *layoutManager = app->layoutManager();
	qMRMLThreeDWidget *threeDWidget = layoutManager->threeDWidget(0);
	qMRMLThreeDView *threeDView = threeDWidget->threeDView();
	this->renderWindow = threeDView->renderWindow();
    this->renderer = this->renderWindow->GetRenderers()->GetFirstRenderer();
    this->iren = this->renderWindow->GetInteractor();

	vtkMRMLScene *mrmlScene = app->mrmlScene();
	
	m_fnode1 = vtkSmartPointer<vtkMRMLAnnotationFiducialNode>::New();

	QString sName1 = tr("F1");
	m_fnode1->SetName(sName1.toLatin1().data());
	double coord1[3] = {0,0,0};
    m_fnode1->AddControlPoint(coord1, 0, 0);
    m_fnode1->SetSelected(0);
    m_fnode1->SetLocked(1);
	mrmlScene->AddNode(m_fnode1);

	m_fnode2 = vtkSmartPointer<vtkMRMLAnnotationFiducialNode>::New();

	QString sName2 = tr("F2");
	m_fnode2->SetName(sName2.toLatin1().data());
	double coord2[3] = {0,0,0};
    m_fnode2->AddControlPoint(coord2, 0, 0);
    m_fnode2->SetSelected(0);
    m_fnode2->SetLocked(1);
	mrmlScene->AddNode(m_fnode2);

	m_fnode3 = vtkSmartPointer<vtkMRMLAnnotationFiducialNode>::New();
	
	QString sName3 = tr("F3");
	m_fnode3->SetName(sName3.toLatin1().data());
    double coord3[3] = {0, 0, 0};
    m_fnode3->AddControlPoint(coord3, 0, 0);
    m_fnode3->SetSelected(0);
    m_fnode3->SetLocked(1);
	mrmlScene->AddNode(m_fnode3);

	QString nodeID;
	char *charNodeID;
	nodeID = "vtkMRMLAnnotationHierarchyNode2";
	charNodeID = nodeID.toLatin1().data();
	vtkMRMLNode *sourceNode = mrmlScene->GetNodeByID(charNodeID);

	QString sName = tr("List_real");
	sourceNode->SetName(sName.toLatin1().data());

	m_pAxialView=new CAxialView();

	m_refreshTimer = new QTimer(this);
	
	m_ProbeMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	m_RefMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
	m_CalibrateMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
	m_ToolMatrix=vtkSmartPointer<vtkMatrix4x4>::New();

	this->m_pivotCalibration = PivotCalibration::New();

	//add observer for initialization events
	this->m_InitializationObserver = InitializationObserverType::New();
	this->m_InitializationObserver->SetCallbackFunction( 
	this, 
	&qSlicerSNSnavExtensionModuleWidget::OnInitializationEvent );

	this->m_pivotCalibration->AddObserver( 
								 PivotCalibration::InitializationFailureEvent(),
											   this->m_InitializationObserver );
	this->m_pivotCalibration->AddObserver( 
								 PivotCalibration::InitializationSuccessEvent(),
											   this->m_InitializationObserver );

	//add observer for the events generated during pivot calibration
	this->m_CalibrationObserver = CalibrationObserverType::New();
	this->m_CalibrationObserver->SetCallbackFunction( 
	this,&qSlicerSNSnavExtensionModuleWidget::OnCalibrationEvent );

	this->m_pivotCalibration->AddObserver( 
                                       PivotCalibration::DataAcquisitionEvent(),
                                                  this->m_CalibrationObserver );
	this->m_pivotCalibration->AddObserver( 
									PivotCalibration::DataAcquisitionEndEvent(),
												  this->m_CalibrationObserver );
	this->m_pivotCalibration->AddObserver( 
									PivotCalibration::CalibrationSuccessEvent(),
												  this->m_CalibrationObserver );
	this->m_pivotCalibration->AddObserver( 
									PivotCalibration::CalibrationFailureEvent(),
												  this->m_CalibrationObserver );

	this->m_TransformToObserver = TransformToObserver::New();
	this->m_RMSEObserver = RMSEObserver::New();
}

//-----------------------------------------------------------------------------
qSlicerSNSnavExtensionModuleWidget::~qSlicerSNSnavExtensionModuleWidget()
{
	//delete m_refreshTimer;
}

void qSlicerSNSnavExtensionModuleWidget::OnSurfacePick()
{
	Q_D(qSlicerSNSnavExtensionModuleWidget);

	vtkMRMLAbstractLogic* l = this->logic();
	vtkSlicerSNSnavExtensionLogic * arLogic = vtkSlicerSNSnavExtensionLogic::SafeDownCast(l);

	if(d->SurfacePick->isChecked())
	{
		arLogic->m_bCollectPoints=true;
	}
	else
	{
		arLogic->m_bCollectPoints=false;
	}
}

void qSlicerSNSnavExtensionModuleWidget::OnPointShow()
{
	Q_D(qSlicerSNSnavExtensionModuleWidget);

	qSlicerApplication *app = qSlicerApplication::application();
	qSlicerLayoutManager *layoutManager = app->layoutManager();
	qMRMLThreeDWidget *threeDWidget = layoutManager->threeDWidget(0);
	qMRMLThreeDView *threeDView = threeDWidget->threeDView();
	
	vtkMRMLAbstractLogic* l = this->logic();
	vtkSlicerSNSnavExtensionLogic * arLogic = vtkSlicerSNSnavExtensionLogic::SafeDownCast(l);

	if(d->PointShow->isChecked())
	{
		arLogic->m_pSurfaceRegistration->HidePoints(threeDView->renderWindow());
	}
	else
	{
		arLogic->m_pSurfaceRegistration->ShowPoints(threeDView->renderWindow());
	}
}

void qSlicerSNSnavExtensionModuleWidget::OnAllPointsDelete()
{
	Q_D(qSlicerSNSnavExtensionModuleWidget);

	qSlicerApplication *app = qSlicerApplication::application();
	qSlicerLayoutManager *layoutManager = app->layoutManager();
	qMRMLThreeDWidget *threeDWidget = layoutManager->threeDWidget(0);
	qMRMLThreeDView *threeDView = threeDWidget->threeDView();
	
	vtkMRMLAbstractLogic* l = this->logic();
	vtkSlicerSNSnavExtensionLogic * arLogic = vtkSlicerSNSnavExtensionLogic::SafeDownCast(l);

	arLogic->m_pSurfaceRegistration->DeleteAllPoints(threeDView->renderWindow()->GetRenderers()->GetFirstRenderer(),threeDView->renderWindow());
}

void qSlicerSNSnavExtensionModuleWidget::OnPrePointDelete()
{
	Q_D(qSlicerSNSnavExtensionModuleWidget);

	qSlicerApplication *app = qSlicerApplication::application();
	qSlicerLayoutManager *layoutManager = app->layoutManager();
	qMRMLThreeDWidget *threeDWidget = layoutManager->threeDWidget(0);
	qMRMLThreeDView *threeDView = threeDWidget->threeDView();
	
	vtkMRMLAbstractLogic* l = this->logic();
	vtkSlicerSNSnavExtensionLogic * arLogic = vtkSlicerSNSnavExtensionLogic::SafeDownCast(l);

	arLogic->m_pSurfaceRegistration->DeletePrePoint(threeDView->renderWindow());
}

void qSlicerSNSnavExtensionModuleWidget::OnPickPoint()
{
	Q_D(qSlicerSNSnavExtensionModuleWidget);
	// now make an annotation
	qSlicerApplication *app = qSlicerApplication::application();
	vtkMRMLScene *mrmlScene = app->mrmlScene();

	//QMessageBox::information(0, tr("Information"),"CubeActor");
		double* fTipPoint=new double[4] ; 
		double* fTipPoint1=new double[4] ;  

		fTipPoint[0]=0.0f;
		fTipPoint[1]=0.0f;
		fTipPoint[2]=0.0f;
		fTipPoint[3]=1.0f;		

		vtkSmartPointer<vtkMatrix4x4> Probe2RefMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
		vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(mrmlScene->GetNodeByID("vtkMRMLLinearTransformNode4"));
		Probe2RefMatrix = transformNode->GetMatrixTransformToParent();
		Probe2RefMatrix->MultiplyPoint(fTipPoint, fTipPoint1);

		QString str;
		str.sprintf("%.2f ,%.2f, %.2f",  fTipPoint1[0],fTipPoint1[1],fTipPoint1[2]);

		d->listWidget->addItem(str);

		if(d->comboPointTool->currentIndex()==0)
			m_fnode1->SetFiducialCoordinates(fTipPoint1[0],fTipPoint1[1],fTipPoint1[2]);
		else if(d->comboPointTool->currentIndex()==1)
			m_fnode2->SetFiducialCoordinates(fTipPoint1[0],fTipPoint1[1],fTipPoint1[2]);
		else if(d->comboPointTool->currentIndex()==2)
		{
			m_fnode3->SetFiducialCoordinates(fTipPoint1[0],fTipPoint1[1],fTipPoint1[2]);
		}
			
		delete[] fTipPoint;
	    delete[] fTipPoint1;
}

void qSlicerSNSnavExtensionModuleWidget::OnSurfaceRegistration()
{
	Q_D(qSlicerSNSnavExtensionModuleWidget);

	qSlicerApplication *app = qSlicerApplication::application();
	qSlicerLayoutManager *layoutManager = app->layoutManager();
	qMRMLThreeDWidget *threeDWidget = layoutManager->threeDWidget(0);
	qMRMLThreeDView *threeDView = threeDWidget->threeDView();	

	vtkMRMLScene *mrmlScene = app->mrmlScene();

	vtkSmartPointer<vtkMatrix4x4> RegisMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
	vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(mrmlScene->GetNodeByID("vtkMRMLModelNode5"));

	vtkMRMLAbstractLogic* l = this->logic();
	vtkSlicerSNSnavExtensionLogic * arLogic = vtkSlicerSNSnavExtensionLogic::SafeDownCast(l);

	vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(mrmlScene->GetNodeByID("vtkMRMLLinearTransformNode5"));
	RegisMatrix = transformNode->GetMatrixTransformToParent();
	arLogic->m_pSurfaceRegistration->Registration((vtkPolyData *) modelNode->GetPolyData(),RegisMatrix,threeDView->renderWindow());	
	transformNode->SetAndObserveMatrixTransformToParent(arLogic->m_pSurfaceRegistration->m_RegisMatrix);

	QMessageBox::information(0, tr("Information"),"Surface Registration Succeed");
}

void qSlicerSNSnavExtensionModuleWidget::OnStartTracking()
 {
	Q_D(qSlicerSNSnavExtensionModuleWidget);

    m_TrackerInitializer = new igstk::TrackerInitializer;
	m_TrackerConfiguration = new igstk::TrackerConfiguration;

	m_TrackerConfiguration->SetTrackerType(igstk::TrackerConfiguration::Polaris);

	igstk::NDITrackerConfiguration *TrackerConfiguration=new igstk::NDITrackerConfiguration;

	// Wired or Wireless tool, port & SROMs
	igstk::NDITrackerToolConfiguration * ProbeTool = new igstk::NDITrackerToolConfiguration;
	ProbeTool->m_WiredTool = 0;
	ProbeTool->m_HasSROM = 1;
    ProbeTool->m_SROMFile = "C:\\Study\\MaxilloNavi140327\\ResourceFiles\\Rom\\NDI_glassprobe.rom";
	ProbeTool->m_IsReference = 0;
	ProbeTool->m_PortNumber=0;	
	TrackerConfiguration->m_TrackerToolList.push_back(ProbeTool);

	igstk::NDITrackerToolConfiguration * ReferenceTool = new igstk::NDITrackerToolConfiguration;
	ReferenceTool->m_WiredTool = 0;
	ReferenceTool->m_HasSROM = 1;
	ReferenceTool->m_SROMFile = "ResourceFiles\\Rom\\tool_reference.rom";
	ReferenceTool->m_IsReference = 0;
	ReferenceTool->m_PortNumber=1; 
	TrackerConfiguration->m_TrackerToolList.push_back(ReferenceTool);

	igstk::NDITrackerToolConfiguration * CalibrationTool = new igstk::NDITrackerToolConfiguration;
	CalibrationTool->m_WiredTool = 0;
	CalibrationTool->m_HasSROM = 1;
	CalibrationTool->m_SROMFile = "ResourceFiles\\Rom\\Calibrate.rom";
	CalibrationTool->m_IsReference = 0;
	CalibrationTool->m_PortNumber=2;
	TrackerConfiguration->m_TrackerToolList.push_back(CalibrationTool);

	igstk::NDITrackerToolConfiguration * SurgicalTool = new igstk::NDITrackerToolConfiguration;
	SurgicalTool->m_WiredTool = 0;
	SurgicalTool->m_HasSROM = 1;
	SurgicalTool->m_SROMFile = "C:\\Study\\MaxilloNavi140327\\ResourceFiles\\Rom\\Surgical_drilljia.rom";
	SurgicalTool->m_IsReference = 0;
	SurgicalTool->m_PortNumber=3;
	TrackerConfiguration->m_TrackerToolList.push_back(SurgicalTool);

	for ( int i = 0; i < 7; i++ ) 
	{
		TrackerConfiguration->m_COMPort=igstk::SerialCommunication::PortNumberType(i);

		m_TrackerConfiguration->SetNDITrackerConfiguration(TrackerConfiguration);
		m_TrackerInitializer->SetTrackerConfiguration( m_TrackerConfiguration );

		if ( m_TrackerInitializer->RequestInitializeTracker() )
		{
			d->START_TRACKING->setEnabled(false);
			d->STOP_TRACKING->setEnabled(true);

			d->CALIBRATE->setEnabled(true);
			d->OBTAIN_ANGLE->setEnabled(true);
			d->STOP_TRACKING->setEnabled(true);

			m_refreshTimer->start(10);

			break;			
		}
	}
 }

void qSlicerSNSnavExtensionModuleWidget::OnStopTracking()
{
    Q_D(qSlicerSNSnavExtensionModuleWidget);
   
	m_refreshTimer->stop();
	m_TrackerInitializer->GetTracker()->RequestStopTracking();
	m_TrackerInitializer->StopAndCloseTracker();	

	d->START_TRACKING->setEnabled(true);
	d->STOP_TRACKING->setEnabled(false);

	d->PROBE_STATUS->setEnabled(false);
	d->REF_STATUS->setEnabled(false);
	d->CALIBRATION_STATUS->setEnabled(false);
	d->SURG_TOOL_STATUS->setEnabled(false);

	d->CALIBRATE->setEnabled(false);
	d->OBTAIN_ANGLE->setEnabled(false);
	d->STOP_TRACKING->setEnabled(false);
}

void qSlicerSNSnavExtensionModuleWidget::timer_refresh()
{
   Q_D(qSlicerSNSnavExtensionModuleWidget);

   igstk::PulseGenerator::CheckTimeouts();	

   if (m_TrackerInitializer->GetNonReferenceToolList()[0]->GetUpdated())
		d->PROBE_STATUS->setEnabled(true);	
   else
		d->PROBE_STATUS->setEnabled(false);

   if (m_TrackerInitializer->GetNonReferenceToolList()[1]->GetUpdated())
		d->REF_STATUS->setEnabled(true);	
   else
		d->REF_STATUS->setEnabled(false);

   if (m_TrackerInitializer->GetNonReferenceToolList()[2]->GetUpdated())
		d->CALIBRATION_STATUS->setEnabled(true);	
   else
		d->CALIBRATION_STATUS->setEnabled(false);

   if (m_TrackerInitializer->GetNonReferenceToolList()[3]->GetUpdated())
		d->SURG_TOOL_STATUS->setEnabled(true);	
	else
		d->SURG_TOOL_STATUS->setEnabled(false);

   ObserverType::Pointer ProbeTransformObserver = ObserverType::New();
   ObserverType::Pointer RefTransformObserver = ObserverType::New();
   ObserverType::Pointer CalibrateTransformObserver = ObserverType::New();
   ObserverType::Pointer ToolTransformObserver = ObserverType::New();

   ProbeTransformObserver->ObserveTransformEventsFrom( m_TrackerInitializer->GetNonReferenceToolList()[0] );
   ProbeTransformObserver->Clear();    
   m_TrackerInitializer->GetNonReferenceToolList()[0]->RequestGetTransformToParent();

   RefTransformObserver->ObserveTransformEventsFrom( m_TrackerInitializer->GetNonReferenceToolList()[1] );
   RefTransformObserver->Clear();
   m_TrackerInitializer->GetNonReferenceToolList()[1]->RequestGetTransformToParent();

   CalibrateTransformObserver->ObserveTransformEventsFrom( m_TrackerInitializer->GetNonReferenceToolList()[2] );
   CalibrateTransformObserver->Clear();
   m_TrackerInitializer->GetNonReferenceToolList()[2]->RequestGetTransformToParent();

   ToolTransformObserver->ObserveTransformEventsFrom( m_TrackerInitializer->GetNonReferenceToolList()[3] );
   ToolTransformObserver->Clear();
   m_TrackerInitializer->GetNonReferenceToolList()[3]->RequestGetTransformToParent();

   if ( ProbeTransformObserver->GotTransform() )
		ProbeTransformObserver->GetTransform().ExportTransform(*m_ProbeMatrix);

   if ( RefTransformObserver->GotTransform() )
		RefTransformObserver->GetTransform().ExportTransform(*m_RefMatrix);

   if ( CalibrateTransformObserver->GotTransform() )
		CalibrateTransformObserver->GetTransform().ExportTransform(*m_CalibrateMatrix);

   if ( ToolTransformObserver->GotTransform() )
		ToolTransformObserver->GetTransform().ExportTransform(*m_ToolMatrix);

   ProbeTransformObserver->Delete();
   RefTransformObserver->Delete();
   CalibrateTransformObserver->Delete();
   ToolTransformObserver->Delete();
}

void qSlicerSNSnavExtensionModuleWidget::OnCalibrate()
{
   Q_D(qSlicerSNSnavExtensionModuleWidget);

   igstk::TrackerTool::Pointer CaliToolTrackerTool;
	if(d->comboCaliTool->currentIndex()==0)
		CaliToolTrackerTool = m_TrackerInitializer->GetNonReferenceToolList()[0];
	else if(d->comboCaliTool->currentIndex()==1)
		CaliToolTrackerTool = m_TrackerInitializer->GetNonReferenceToolList()[3];		

	this->m_pivotCalibration->RequestInitialize(300,CaliToolTrackerTool );
	this->m_pivotCalibration->RequestComputeCalibration();

}

void qSlicerSNSnavExtensionModuleWidget::OnCalibrationEvent( itk::Object *caller, 
                                                const itk::EventObject & event )
{
  Q_D(qSlicerSNSnavExtensionModuleWidget);

  if( const PivotCalibration::DataAcquisitionEvent *daevt =
       dynamic_cast< const PivotCalibration::DataAcquisitionEvent * > (&event) )
    {	
		m_pAxialView->m_nProgress = (int) (daevt->Get()*100);
		emit ProgressUpdate(m_pAxialView->m_nProgress);   
    }
  else if( dynamic_cast< 
                  const PivotCalibration::DataAcquisitionEndEvent * > (&event) )
    {
		emit ProgressUpdate(0);		
    }
  else if( const PivotCalibration::CalibrationFailureEvent *evt = 
           dynamic_cast< 
                  const PivotCalibration::CalibrationFailureEvent * > (&event) )
    {		
		QMessageBox::information(0, tr("Information"),"Calibration failed");
		d->progressBar->hide();    
   }
  //calibration succeeded, get all the information 
  //(Transformation, Pivot Point, RMSE) and display it
  else if( dynamic_cast< 
                  const PivotCalibration::CalibrationSuccessEvent * > (&event) )
    {
		PivotCalibration* calib = dynamic_cast< PivotCalibration *> (caller);

		//get the transformation
		unsigned long observerID = calib->AddObserver( 
										  igstk::CoordinateSystemTransformToEvent(),
													  this->m_TransformToObserver );
		calib->RequestCalibrationTransform();
		calib->RemoveObserver( observerID );
		igstk::Transform transform = 
					   this->m_TransformToObserver->GetTransformTo().GetTransform();
		igstk::Transform::VersorType v = transform.GetRotation();
		igstk::Transform::VectorType t = transform.GetTranslation();

		QString str;
		str.sprintf("%.2f,%.2f,%.2f",t[0],t[1],t[2]);

		if(d->comboCaliTool->currentIndex()==0)
		{
			m_fProbeTipOffset[0]=t[0];m_fProbeTipOffset[1]=t[1];m_fProbeTipOffset[2]=t[2];
		}
		else
		{
			m_fToolTipOffset[0]=t[0];m_fToolTipOffset[1]=t[1];m_fToolTipOffset[2]=t[2];
		}
		
		d->listWidget->addItem(str);
		d->progressBar->hide();
	
		observerID = calib->AddObserver( igstk::DoubleTypeEvent(),
															 this->m_RMSEObserver );
		calib->RequestCalibrationRMSE();

		str.sprintf("RMSE: %.2f",this->m_RMSEObserver->GetRMSE());
		d->listWidget->addItem(str);
    }
}

void qSlicerSNSnavExtensionModuleWidget::OnInitializationEvent( itk::Object * itkNotUsed(caller),
                                             const itk::EventObject & event )
{
   Q_D(qSlicerSNSnavExtensionModuleWidget);

   if( dynamic_cast< const 
              igstk::PivotCalibration::InitializationSuccessEvent * > (&event) )
    {
		d->listWidget->addItem("InitializationSuccess");
		d->progressBar->show();		
    }
  else if( dynamic_cast< 
        const igstk::PivotCalibration::InitializationFailureEvent * > (&event) )
    {
		QMessageBox::information(0, tr("Information"),"InitializationFailure");    
    }
}

void qSlicerSNSnavExtensionModuleWidget::advanceProgressBar(int nProgress)
{
  Q_D(qSlicerSNSnavExtensionModuleWidget);

  d->progressBar->setValue(nProgress);
}

void qSlicerSNSnavExtensionModuleWidget::OnObtainAngle()
{
  Q_D(qSlicerSNSnavExtensionModuleWidget);

  if(d->SURG_TOOL_STATUS->isEnabled() && d->CALIBRATION_STATUS->isEnabled())
	{
		double* fNormalVector=new double[4] ; 
		double* fTipPoint=new double[4] ; 	

		fNormalVector[0]=1.0f;
		fNormalVector[1]=0.0f;
		fNormalVector[2]=0.0f;
		fNormalVector[3]=0.0f;		

		vtkSmartPointer<vtkMatrix4x4> TransformMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
		m_ToolMatrix->Invert(m_ToolMatrix,TransformMatrix);
		TransformMatrix->Multiply4x4(TransformMatrix,m_CalibrateMatrix,TransformMatrix);
		TransformMatrix->MultiplyPoint(fNormalVector, fTipPoint);
		
		m_fToolAxis[0]=fTipPoint[0];m_fToolAxis[1]=fTipPoint[1];m_fToolAxis[2]=fTipPoint[2];

		QString str;
		str.sprintf("Calibrated Angle:%.3f,%.3f,%.3f",m_fToolAxis[0],m_fToolAxis[1],m_fToolAxis[2]);
		
		d->listWidget->addItem(str);

		delete[] fNormalVector;
		delete[] fTipPoint;		
	}
	else
	{
		QMessageBox::information(0, tr("Information"),"The calibration block or tool is out of the view!");
	}
}

void qSlicerSNSnavExtensionModuleWidget::OnCreateProbeXml()
{
	Q_D(qSlicerSNSnavExtensionModuleWidget);

	m_ProbeXml.sprintf("%d %d %d %d %.2f %.2f %.2f",0,0,0,1,m_fProbeTipOffset[0],m_fProbeTipOffset[1],m_fProbeTipOffset[2]);

	QString m_filename;
	m_filename = "C:\\IGSTK-4.4_build\\bin\\Release\\ProbeCalibration.xml";
	if (m_filename.isEmpty())
		return;
	QFile mfile(m_filename);	
	if (!mfile.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("ProbePoint"),
			tr("Cannot write file %1:\n%2.")
			.arg(m_filename)
			.arg(mfile.errorString()));
		return;
	}

	QXmlStreamWriter xmlWriter(&mfile);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.setCodec("ISO 8859-1");
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("precomputed_transform");
	xmlWriter.writeCharacters("\n\n");

	xmlWriter.writeTextElement("description","\n Probe (sptr) calibration\n");
	xmlWriter.writeCharacters("\n\n");
	xmlWriter.writeTextElement("computation_date","\n Thursday July 4 12:00:00 1776\n");
	xmlWriter.writeCharacters("\n\n");

	xmlWriter.writeStartElement("transformation");
	xmlWriter.writeAttribute("estimation_error","0.5");
	xmlWriter.writeCharacters(m_ProbeXml.append("\n").prepend("\n"));
	xmlWriter.writeEndElement();

	xmlWriter.writeCharacters("\n\n");
	xmlWriter.writeEndElement();

	xmlWriter.writeEndDocument();	
	mfile.close();
}

void qSlicerSNSnavExtensionModuleWidget::OnCreateToolXml()
{
   Q_D(qSlicerSNSnavExtensionModuleWidget);

   Euler2Quaternion();

   m_ToolXml.sprintf("%.3f %.3f %.3f %.3f %.2f %.2f %.2f",m_fToolAxisQuaternion[0],m_fToolAxisQuaternion[1],m_fToolAxisQuaternion[2],m_fToolAxisQuaternion[3],m_fToolTipOffset[0],m_fToolTipOffset[1],m_fToolTipOffset[2]);

	QString m_filename;
	m_filename = "C:\\IGSTK-4.4_build\\bin\\Release\\PinCalibration.xml";
	if (m_filename.isEmpty())
		return;
	QFile mfile(m_filename);	
	if (!mfile.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("ToolPoint"),
			tr("Cannot write file %1:\n%2.")
			.arg(m_filename)
			.arg(mfile.errorString()));
		return;
	}

	QXmlStreamWriter xmlWriter(&mfile);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.setCodec("ISO 8859-1");
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("precomputed_transform");
	xmlWriter.writeCharacters("\n\n");

	xmlWriter.writeTextElement("description","\n Probe (sptr) calibration\n");
	xmlWriter.writeCharacters("\n\n");
	xmlWriter.writeTextElement("computation_date","\n Thursday July 4 12:00:00 1776\n");
	xmlWriter.writeCharacters("\n\n");

	xmlWriter.writeStartElement("transformation");
	xmlWriter.writeAttribute("estimation_error","0.5");
	xmlWriter.writeCharacters(m_ToolXml.append("\n").prepend("\n"));
	xmlWriter.writeEndElement();

	xmlWriter.writeCharacters("\n\n");
	xmlWriter.writeEndElement();

	xmlWriter.writeEndDocument();	
	mfile.close();
}

void qSlicerSNSnavExtensionModuleWidget::Euler2Quaternion()
{
   Q_D(qSlicerSNSnavExtensionModuleWidget);
  
    double* f_UVector=new double[4] ;

	f_UVector[0]= (-1)*(-1)*m_fToolAxis[1];
	f_UVector[1]= (-1)*m_fToolAxis[0];
	f_UVector[2]= 0.0f;
	f_UVector[3]= 0.0f;

	double i;
	i = pow(m_fToolAxis[0],2)+pow(m_fToolAxis[1],2)+pow(m_fToolAxis[2],2);

	double n;
	n = (-1)*m_fToolAxis[2]/sqrt(i);

	double half_degree;
	half_degree = acos(n)/2;

	m_fToolAxisQuaternion[0] = sin(half_degree)*f_UVector[0];
	m_fToolAxisQuaternion[1] = sin(half_degree)*f_UVector[1];
	m_fToolAxisQuaternion[2] = sin(half_degree)*f_UVector[2];
	m_fToolAxisQuaternion[3] = cos(half_degree);
}

void qSlicerSNSnavExtensionModuleWidget::OnSTART_OPENIGT()
{	
   WinExec("C:\\IGSTK-4.4_build\\bin\\Release\\wizard-OpenIGSTK.bat",SW_SHOW);
}
//-----------------------------------------------------------------------------
void qSlicerSNSnavExtensionModuleWidget::setup()
{
  Q_D(qSlicerSNSnavExtensionModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
										  
  QObject::connect(d->PickPoint, SIGNAL(clicked()), this, SLOT(OnPickPoint()));
  QObject::connect(d->SurfacePick, SIGNAL(clicked()), this, SLOT(OnSurfacePick()));
  QObject::connect(d->SurfaceRegistration, SIGNAL(clicked()), this, SLOT(OnSurfaceRegistration()));
  QObject::connect(d->PointShow, SIGNAL(clicked()), this, SLOT(OnPointShow()));
  QObject::connect(d->AllPointsDelete, SIGNAL(clicked()), this, SLOT(OnAllPointsDelete()));
  QObject::connect(d->PrePointDelete, SIGNAL(clicked()), this, SLOT(OnPrePointDelete()));
  QObject::connect(d->CALIBRATE, SIGNAL(clicked()), this, SLOT(OnCalibrate()));
  QObject::connect(d->START_TRACKING, SIGNAL(clicked()), this, SLOT(OnStartTracking()));
  QObject::connect(d->STOP_TRACKING, SIGNAL(clicked()), this, SLOT(OnStopTracking()));
  QObject::connect(m_refreshTimer,SIGNAL(timeout()),this,SLOT(timer_refresh()));
  QObject::connect(this, SIGNAL(ProgressUpdate(int)), this, SLOT(advanceProgressBar(int)));
  QObject::connect(d->OBTAIN_ANGLE, SIGNAL(clicked()), this, SLOT(OnObtainAngle()));
  QObject::connect(d->CreateProbeXml, SIGNAL(clicked()), this, SLOT(OnCreateProbeXml()));
  QObject::connect(d->CreateToolXml, SIGNAL(clicked()), this, SLOT(OnCreateToolXml()));
  QObject::connect(d->START_OPENIGT, SIGNAL(clicked()), this, SLOT(OnSTART_OPENIGT()));
								
  d->progressBar->hide(); 

}

