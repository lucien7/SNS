#include "AxialView.h"

bool IsChineseIncluded(QString str);

CAxialView::CAxialView(void)
{
	Progress=vtkSmartPointer<vtkProgress>::New();

	m_CommandObserver    = CommandType::New();
	m_CommandObserver->SetCallbackFunction( this, &CAxialView::ProgressUpdateFunc);
	m_nProgress=0;
}

void CAxialView::ProgressUpdateFunc( itk::Object * caller, const itk::EventObject & event )
{
	itk::ProcessObject::Pointer process =
			  dynamic_cast< itk::ProcessObject *>( caller );
	

	if( typeid( itk::EndEvent ) == typeid( event ) )
	{
		m_nProgress=0;
	}

	if( typeid( itk::ProgressEvent ) == typeid( event ) )
	{
		m_nProgress = (int) (process->GetProgress()*100);		
	}

	emit ProgressUpdate(m_nProgress);
}

CAxialView::~CAxialView(void)
{
}
