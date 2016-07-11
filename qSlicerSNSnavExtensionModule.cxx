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
#include <QtPlugin>

// ExtensionTemplate Logic includes
#include <vtkSlicerSNSnavExtensionLogic.h>

// ExtensionTemplate includes
#include "qSlicerSNSnavExtensionModule.h"
#include "qSlicerSNSnavExtensionModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSNSnavExtensionModule, qSlicerSNSnavExtensionModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSNSnavExtensionModulePrivate
{
public:
  qSlicerSNSnavExtensionModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSNSnavExtensionModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSNSnavExtensionModulePrivate::qSlicerSNSnavExtensionModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSNSnavExtensionModule methods

//-----------------------------------------------------------------------------
qSlicerSNSnavExtensionModule::qSlicerSNSnavExtensionModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSNSnavExtensionModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSNSnavExtensionModule::~qSlicerSNSnavExtensionModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSNSnavExtensionModule::helpText()const
{
  return "This is a loadable module bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerSNSnavExtensionModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSNSnavExtensionModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSNSnavExtensionModule::icon()const
{
  return QIcon(":/Icons/SNSnavExtension.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSNSnavExtensionModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSNSnavExtensionModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerSNSnavExtensionModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerSNSnavExtensionModule::createWidgetRepresentation()
{
  return new qSlicerSNSnavExtensionModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSNSnavExtensionModule::createLogic()
{
  return vtkSlicerSNSnavExtensionLogic::New();
}
