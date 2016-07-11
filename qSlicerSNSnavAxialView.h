#ifndef AXIALVIEW_H
#define AXIALVIEW_H

#include <QApplication>
#include <QtGui>

#include <vtkSmartPointer.h>

#include "vtkProgress.h"

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkVTKImageExport.h> 

#include <itkGDCMImageIO.h> 
#include <itkImageSeriesReader.h> 
#include <itkGDCMSeriesFileNames.h> 

#include <itkImageIOBase.h> 
#include <itkImageIORegion.h> 

#include <vtkImageImport.h>

#include "AxialView.h"

class CAxialView: public QObject
{
	Q_OBJECT

public:
	CAxialView(void);
public:
	~CAxialView(void);

public:

	vtkSmartPointer<vtkProgress> Progress;

	typedef itk::MemberCommand< CAxialView >  CommandType;
	typedef itk::SmartPointer<CommandType> CommandTypePointer; 
	CommandTypePointer	m_CommandObserver;
	void ProgressUpdateFunc( itk::Object * caller, const itk::EventObject & event );

	int m_nProgress;

	signals:
    void ProgressUpdate(int nProgress);

public:

	
};

#endif