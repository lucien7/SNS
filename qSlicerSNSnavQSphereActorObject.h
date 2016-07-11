#ifndef QSPHEREACTOROBJECT_H
#define QSPHEREACTOROBJECT_H

#include <QApplication>
#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkFollower.h>
#include <vtkSmartPointer.h>

class QSphereActorObject :
	public QObject
{
public:
	vtkSmartPointer<vtkActor> sphereActor;
	vtkSmartPointer<vtkSphereSource> sphere;
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper;
	double m_fPosition[3];
	vtkSmartPointer<vtkFollower> labelActor;

public:
	QSphereActorObject(){}
public:
	~QSphereActorObject(){}
};

#endif