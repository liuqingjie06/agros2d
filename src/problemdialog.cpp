// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "problemdialog.h"

#include "gui.h"
#include "scene.h"
#include "scenesolution.h"
#include "scripteditordialog.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"

FieldSelectDialog::FieldSelectDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("FieldSelectDialog::FieldSelectDialog()");

    setWindowTitle(tr("Select field"));
    setModal(true);

    m_fieldId = "";

    lstFields = new QListWidget(this);

    std::map<std::string, std::string> modules = availableModules();
    for (std::map<std::string, std::string>::iterator it = modules.begin();
         it != modules.end(); ++it)
    {
        // add only missing fields
        if (!Util::scene()->fieldInfos().contains(QString::fromStdString(it->first)))
        {
            QListWidgetItem *item = new QListWidgetItem(lstFields);
            item->setText(QString::fromStdString(it->second));
            item->setData(Qt::UserRole, QString::fromStdString(it->first));

            lstFields->addItem(item);
        }
    }

    connect(lstFields, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(doItemDoubleClicked(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemPressed(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));

    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstFields);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (lstFields->count() > 0)
    {
        lstFields->setCurrentRow(0);
        doItemSelected(lstFields->currentItem());
    }
}

void FieldSelectDialog::doAccept()
{
    logMessage("FieldSelectDialog::doAccept()");

    accept();
}

void FieldSelectDialog::doReject()
{
    logMessage("FieldSelectDialog::doReject()");

    reject();
}

int FieldSelectDialog::showDialog()
{
    return exec();
}

void FieldSelectDialog::doItemSelected(QListWidgetItem *item)
{
    m_fieldId = item->data(Qt::UserRole).toString();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FieldSelectDialog::doItemDoubleClicked(QListWidgetItem *item)
{
    if (lstFields->currentItem())
    {
        m_fieldId = lstFields->currentItem()->data(Qt::UserRole).toString();
        accept();
    }
}

// ********************************************************************************************************

FieldWidget::FieldWidget(const ProblemInfo *problemInfo, FieldInfo *fieldInfo, QWidget *parent)
    : QWidget(parent), problemInfo(problemInfo), fieldInfo(fieldInfo)
{
    createContent();
    load();
}

void FieldWidget::createContent()
{
    // equations
    lblEquationPixmap = new QLabel("");
    lblEquationPixmap->setMinimumHeight(50);

    cmbAdaptivityType = new QComboBox();
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(1);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivityTolerance = new SLineEditDouble(1);

    // mesh
    txtNumberOfRefinements = new QSpinBox(this);
    txtNumberOfRefinements->setMinimum(0);
    txtNumberOfRefinements->setMaximum(5);
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);
    cmbMeshType = new QComboBox();

    // weak forms
    cmbWeakForms = new QComboBox();

    // transient
    cmbAnalysisType = new QComboBox();
    txtTransientInitialCondition = new ValueLineEdit();

    // linearity
    cmbLinearityType = new QComboBox();
    txtNonlinearSteps = new QSpinBox(this);
    txtNonlinearSteps->setMinimum(1);
    txtNonlinearSteps->setMaximum(100);
    txtNonlinearTolerance = new SLineEditDouble(1);

    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));

    connect(cmbLinearityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearityTypeChanged(int)));

    // fill combobox
    fillComboBox();

    int minWidth = 130;

    // table
    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->setColumnMinimumWidth(0, minWidth);
    layoutTable->setColumnStretch(1, 1);
    layoutTable->addWidget(new QLabel(tr("Type of analysis:")), 5, 0);
    layoutTable->addWidget(cmbAnalysisType, 5, 1);
    layoutTable->addWidget(new QLabel(tr("Adaptivity:")), 6, 0);
    layoutTable->addWidget(cmbAdaptivityType, 6, 1);
    layoutTable->addWidget(new QLabel(tr("Weak forms:")), 9, 0);
    layoutTable->addWidget(cmbWeakForms, 9, 1);

    // transient analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, minWidth);
    layoutTransientAnalysis->setColumnStretch(1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial condition:")), 2, 0);
    layoutTransientAnalysis->addWidget(txtTransientInitialCondition, 2, 1);

    QGroupBox *grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // harmonic analysis
    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->setColumnMinimumWidth(0, minWidth);
    layoutMesh->setColumnStretch(1, 1);
    layoutMesh->addWidget(new QLabel(tr("Mesh type:")), 0, 0);
    layoutMesh->addWidget(cmbMeshType, 0, 1);
    layoutMesh->addWidget(new QLabel(tr("Number of refinements:")), 1, 0);
    layoutMesh->addWidget(txtNumberOfRefinements, 1, 1);
    layoutMesh->addWidget(new QLabel(tr("Polynomial order:")), 2, 0);
    layoutMesh->addWidget(txtPolynomialOrder, 2, 1);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh parameters"));
    grpMesh->setLayout(layoutMesh);

    // adaptivity
    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->setColumnMinimumWidth(0, minWidth);
    layoutAdaptivity->setColumnStretch(1, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Adaptivity steps:")), 0, 0);
    layoutAdaptivity->addWidget(txtAdaptivitySteps, 0, 1);
    layoutAdaptivity->addWidget(new QLabel(tr("Adaptivity tolerance (%):")), 1, 0);
    layoutAdaptivity->addWidget(txtAdaptivityTolerance, 1, 1);

    QGroupBox *grpAdaptivity = new QGroupBox(tr("Adaptivity"));
    grpAdaptivity->setLayout(layoutAdaptivity);

    // linearity
    QGridLayout *layoutLinearity = new QGridLayout();
    layoutLinearity->setColumnMinimumWidth(0, minWidth);
    layoutLinearity->setColumnStretch(1, 1);
    layoutLinearity->addWidget(new QLabel(tr("Linearity:")), 0, 0);
    layoutLinearity->addWidget(cmbLinearityType, 0, 1);
    layoutLinearity->addWidget(new QLabel(tr("Tolerance (%):")), 1, 0);
    layoutLinearity->addWidget(txtNonlinearTolerance, 1, 1);
    layoutLinearity->addWidget(new QLabel(tr("Steps:")), 2, 0);
    layoutLinearity->addWidget(txtNonlinearSteps, 2, 1);

    QGroupBox *grpLinearity = new QGroupBox(tr("Newton solver"));
    grpLinearity->setLayout(layoutLinearity);
    // grpLinearity->setVisible(Util::config()->showExperimentalFeatures);

    // left
    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addLayout(layoutTable);
    layoutLeft->addWidget(grpLinearity);
    layoutLeft->addStretch();

    // right
    QVBoxLayout *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpMesh);
    layoutRight->addWidget(grpAdaptivity);
    layoutRight->addWidget(grpTransientAnalysis);
    layoutRight->addStretch();

    // both
    QHBoxLayout *layoutPanel = new QHBoxLayout();
    layoutPanel->addLayout(layoutLeft);
    layoutPanel->addLayout(layoutRight);

    // equation
    QGridLayout *layoutEquation = new QGridLayout();
    layoutEquation->setColumnMinimumWidth(0, minWidth);
    layoutEquation->setColumnStretch(1, 1);
    layoutEquation->addWidget(new QLabel(tr("Equation:")), 0, 0);
    layoutEquation->addWidget(lblEquationPixmap, 0, 1, 1, 1, Qt::AlignLeft);

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addLayout(layoutEquation);
    layoutProblem->addLayout(layoutPanel);

    setLayout(layoutProblem);

    setMinimumSize(sizeHint());
}

void FieldWidget::fillComboBox()
{
    logMessage("ProblemDialog::fillComboBox()");

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_None), AdaptivityType_None);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_H), AdaptivityType_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_P), AdaptivityType_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityType_HP), AdaptivityType_HP);

    cmbMeshType->addItem(meshTypeString(MeshType_Triangle), MeshType_Triangle);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadFineDivision), MeshType_QuadFineDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadRoughDivision), MeshType_QuadRoughDivision);
    cmbMeshType->addItem(meshTypeString(MeshType_QuadJoin), MeshType_QuadJoin);

    cmbWeakForms->clear();
    cmbWeakForms->addItem(weakFormsTypeString(WeakFormsType_Compiled), WeakFormsType_Compiled);
    cmbWeakForms->addItem(weakFormsTypeString(WeakFormsType_Interpreted), WeakFormsType_Interpreted);
}

void FieldWidget::load()
{
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(fieldInfo->adaptivityType));
    txtAdaptivitySteps->setValue(fieldInfo->adaptivitySteps);
    txtAdaptivityTolerance->setValue(fieldInfo->adaptivityTolerance);
    // weakforms
    cmbWeakForms->setCurrentIndex(cmbWeakForms->findData(fieldInfo->weakFormsType));
    //mesh
    txtNumberOfRefinements->setValue(fieldInfo->numberOfRefinements);
    txtPolynomialOrder->setValue(fieldInfo->polynomialOrder);
    cmbMeshType->setCurrentIndex(cmbMeshType->findData(fieldInfo->meshType));
    // transient
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(fieldInfo->analysisType()));
    txtTransientInitialCondition->setValue(fieldInfo->initialCondition);
    // linearity
    cmbLinearityType->setCurrentIndex(cmbLinearityType->findData(fieldInfo->linearityType));
    txtNonlinearSteps->setValue(fieldInfo->nonlinearSteps);
    txtNonlinearTolerance->setValue(fieldInfo->nonlinearTolerance);

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
    refresh();
}

bool FieldWidget::save()
{    
    fieldInfo->setAnalysisType((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt());

    fieldInfo->initialCondition = txtTransientInitialCondition->value();

    fieldInfo->numberOfRefinements = txtNumberOfRefinements->value();
    fieldInfo->polynomialOrder = txtPolynomialOrder->value();
    fieldInfo->meshType = (MeshType) cmbMeshType->itemData(cmbMeshType->currentIndex()).toInt();
    fieldInfo->adaptivityType = (AdaptivityType) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt();
    fieldInfo->adaptivitySteps = txtAdaptivitySteps->value();
    fieldInfo->adaptivityTolerance = txtAdaptivityTolerance->value();

    fieldInfo->linearityType = (LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt();
    fieldInfo->nonlinearSteps = txtNonlinearSteps->value();
    fieldInfo->nonlinearTolerance = txtNonlinearTolerance->value();

    fieldInfo->weakFormsType = (WeakFormsType) cmbWeakForms->itemData(cmbWeakForms->currentIndex()).toInt();

    return true;
}

void FieldWidget::refresh()
{
    // store analysis type
    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();

    std::map<std::string, std::string> analyses = availableAnalyses(fieldInfo->fieldId().toStdString());
    for (std::map<std::string, std::string>::iterator it = analyses.begin(); it != analyses.end(); ++it)
        cmbAnalysisType->addItem(QString::fromStdString(it->second), analysisTypeFromStringKey(QString::fromStdString(it->first)));

    // restore analysis type
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(analysisType));
    if (cmbAnalysisType->currentIndex() == -1) cmbAnalysisType->setCurrentIndex(0);
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());

    // nonlinearity
    cmbLinearityType->clear();
    cmbLinearityType->addItem(linearityTypeString(LinearityType_Linear), LinearityType_Linear);
    // if (hermesField->hasNonlinearity())
    {
        cmbLinearityType->addItem(linearityTypeString(LinearityType_Picard), LinearityType_Picard);
        cmbLinearityType->addItem(linearityTypeString(LinearityType_Newton), LinearityType_Newton);
    }
    doLinearityTypeChanged(cmbLinearityType->currentIndex());

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

void FieldWidget::doAnalysisTypeChanged(int index)
{
    logMessage("ProblemDialog::doAnalysisTypeChanged()");

    txtTransientInitialCondition->setEnabled((AnalysisType) cmbAnalysisType->itemData(index).toInt() == AnalysisType_Transient);

    doShowEquation();
}

void FieldWidget::doShowEquation()
{
    readPixmap(lblEquationPixmap,
               QString(":/images/equations/%1/%1_%2.png")
               .arg(fieldInfo->fieldId())
               .arg(analysisTypeToStringKey((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt())));
}

void FieldWidget::doAdaptivityChanged(int index)
{
    logMessage("ProblemDialog::doAdaptivityChanged()");

    txtAdaptivitySteps->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
    txtAdaptivityTolerance->setEnabled((AdaptivityType) cmbAdaptivityType->itemData(index).toInt() != AdaptivityType_None);
}

void FieldWidget::doLinearityTypeChanged(int index)
{
    logMessage("ProblemDialog::doLinearityTypeChanged()");

    txtNonlinearSteps->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearTolerance->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
}

// ********************************************************************************************

ProblemDialog::ProblemDialog(ProblemInfo *problemInfo,
                             QMap<QString, FieldInfo *> fieldInfos,
                             bool isNewProblem,
                             QWidget *parent) : QDialog(parent)
{
    logMessage("ProblemDialog::ProblemDialog()");

    m_isNewProblem = isNewProblem;
    m_problemInfo = problemInfo;
    m_fieldInfos = fieldInfos;

    setWindowTitle(tr("Problem properties"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

int ProblemDialog::showDialog()
{
    logMessage("ProblemDialog::showDialog()");

    return exec();
}

void ProblemDialog::createControls()
{
    logMessage("ProblemDialog::createControls()");

    // tab
    QTabWidget *tabType = new QTabWidget();
    tabType->addTab(createControlsGeneral(), icon(""), tr("General"));
    tabType->addTab(createControlsStartupScript(), icon(""), tr("Startup script"));
    tabType->addTab(createControlsDescription(), icon(""), tr("Description"));

    // dialog buttons
    QPushButton *btnAddField = new QPushButton(tr("Add field"));
    btnAddField->setDefault(false);
    btnAddField->setVisible(false);
    connect(btnAddField, SIGNAL(clicked()), this, SLOT(doAddField()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnAddField, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);   

    refresh();
}

QWidget *ProblemDialog::createControlsGeneral()
{
    logMessage("ProblemDialog::createControlsGeneral()");

    // problem
    cmbCoordinateType = new QComboBox();
    txtName = new QLineEdit("");
    dtmDate = new QDateTimeEdit();
    dtmDate->setDisplayFormat("dd.MM.yyyy");
    dtmDate->setCalendarPopup(true);

    cmbMatrixSolver = new QComboBox();

    // harmonic
    txtFrequency = new SLineEditDouble();

    // transient
    txtTransientTimeStep = new ValueLineEdit();
    txtTransientTimeTotal = new ValueLineEdit();
    lblTransientSteps = new QLabel("0");

    connect(txtTransientTimeStep, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(editingFinished()), this, SLOT(doTransientChanged()));

    // fill combobox
    fillComboBox();

    int minWidth = 130;

    // table
    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->setColumnMinimumWidth(0, minWidth);
    layoutTable->setColumnStretch(1, 1);
    layoutTable->addWidget(new QLabel(tr("Date:")), 2, 0);
    layoutTable->addWidget(dtmDate, 2, 1);
    layoutTable->addWidget(new QLabel(tr("Coordinate type:")), 4, 0);
    layoutTable->addWidget(cmbCoordinateType, 4, 1);
    layoutTable->addWidget(new QLabel(tr("Linear solver:")), 8, 0);
    layoutTable->addWidget(cmbMatrixSolver, 8, 1);

    // harmonic analysis
    QGridLayout *layoutHarmonicAnalysis = new QGridLayout();
    layoutHarmonicAnalysis->setColumnMinimumWidth(0, minWidth);
    layoutHarmonicAnalysis->addWidget(new QLabel(tr("Frequency (Hz):")), 0, 0);
    layoutHarmonicAnalysis->addWidget(txtFrequency, 0, 1);

    QGroupBox *grpHarmonicAnalysis = new QGroupBox(tr("Harmonic analysis"));
    grpHarmonicAnalysis->setLayout(layoutHarmonicAnalysis);

    // harmonic analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, minWidth);
    layoutTransientAnalysis->setColumnStretch(1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Time step (s):")), 0, 0);
    layoutTransientAnalysis->addWidget(txtTransientTimeStep, 0, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Total time (s):")), 1, 0);
    layoutTransientAnalysis->addWidget(txtTransientTimeTotal, 1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Steps:")), 3, 0);
    layoutTransientAnalysis->addWidget(lblTransientSteps, 3, 1);

    QGroupBox *grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // left
    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addLayout(layoutTable);
    layoutLeft->addStretch();

    // right
    QVBoxLayout *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpHarmonicAnalysis);
    layoutRight->addWidget(grpTransientAnalysis);
    layoutRight->addStretch();

    // both
    QHBoxLayout *layoutPanel = new QHBoxLayout();
    layoutPanel->addLayout(layoutLeft);
    layoutPanel->addLayout(layoutRight);

    // name
    QGridLayout *layoutName = new QGridLayout();
    layoutName->setColumnMinimumWidth(0, minWidth);
    layoutName->setColumnStretch(1, 1);
    layoutName->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutName->addWidget(txtName, 0, 1);

    // fields
    tabFields = new QTabWidget(this);
    refresh();

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addLayout(layoutName);
    layoutProblem->addLayout(layoutPanel);
    layoutProblem->addWidget(tabFields);

    QWidget *widMain = new QWidget();
    widMain->setLayout(layoutProblem);

    return widMain;
}

QWidget *ProblemDialog::createControlsStartupScript()
{
    logMessage("ProblemDialog::createControlsStartupScript()");

    txtStartupScript = new ScriptEditor(this);

    QVBoxLayout *layoutStartup = new QVBoxLayout();
    layoutStartup->addWidget(txtStartupScript);

    QWidget *widStartup = new QWidget();
    widStartup->setLayout(layoutStartup);

    return widStartup;
}

QWidget *ProblemDialog::createControlsDescription()
{
    logMessage("ProblemDialog::createControlsDescription()");

    txtDescription = new QTextEdit(this);
    txtDescription->setAcceptRichText(false);

    QVBoxLayout *layoutDescription = new QVBoxLayout();
    layoutDescription->addWidget(txtDescription);

    QWidget *widDescription = new QWidget();
    widDescription->setLayout(layoutDescription);

    return widDescription;
}

void ProblemDialog::fillComboBox()
{
    logMessage("ProblemDialog::fillComboBox()");

    cmbCoordinateType->clear();
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Planar), CoordinateType_Planar);
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Axisymmetric), CoordinateType_Axisymmetric);

    cmbMatrixSolver->addItem(matrixSolverTypeString(Hermes::SOLVER_UMFPACK), Hermes::SOLVER_UMFPACK);
}

void ProblemDialog::refresh()
{
    tabFields->clear();

    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
        tabFields->addTab(new FieldWidget(m_problemInfo, fieldInfo, tabFields),
                          QString::fromStdString(fieldInfo->module()->name));

    setMinimumSize(sizeHint());
    resize(sizeHint());
}

void ProblemDialog::load()
{
    logMessage("ProblemDialog::load()");

    // main
    txtName->setText(m_problemInfo->name);
    dtmDate->setDate(m_problemInfo->date);
    cmbCoordinateType->setCurrentIndex(cmbCoordinateType->findData(m_problemInfo->coordinateType));
    // harmonic magnetic
    txtFrequency->setValue(m_problemInfo->frequency);
    // transient
    txtTransientTimeStep->setValue(m_problemInfo->timeStep);
    txtTransientTimeTotal->setValue(m_problemInfo->timeTotal);
    // matrix solver
    cmbMatrixSolver->setCurrentIndex(cmbMatrixSolver->findData(m_problemInfo->matrixSolver));
    // startup
    txtStartupScript->setPlainText(m_problemInfo->scriptStartup);
    // description
    txtDescription->setPlainText(m_problemInfo->description);

    doTransientChanged();
}

bool ProblemDialog::save()
{
    logMessage("ProblemDialog::save()");

    // physical field type
    //    if (Util::scene()->problemInfo()->fieldId() !=
    //            cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString().toStdString())
    //    {
    //        if (!this->m_isNewProblem)
    //        {
    //            if (Util::scene()->boundaries.count() != 1 || Util::scene()->materials.count() != 1)
    //            {
    //                if (QMessageBox::question(this, tr("Change physical field type"), tr("Are you sure change physical field type?"), tr("&Yes"), tr("&No")) == 1)
    //                    return false;
    //            }
    //        }

    //        if (Util::scene()->sceneSolution()->isSolved())
    //            Util::scene()->doClearSolution();

    //        m_problemInfo->setModule(moduleFactory(cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString().toStdString(),
    //                                               (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt(),
    //                                               (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt(),
    //                                               (cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString() == "custom"
    //                                                ? Util::scene()->problemInfo()->fileName.left(Util::scene()->problemInfo()->fileName.size() - 4) + ".xml" : "").toStdString()));

    //        for (int i = 1; i < Util::scene()->boundaries.count(); i++)
    //        {
    //            Util::scene()->replaceBoundary(Util::scene()->boundaries[1]);
    //        }

    //        for (int i = 1; i < Util::scene()->materials.count(); i++)
    //        {
    //            Util::scene()->replaceMaterial(Util::scene()->materials[1]);
    //        }
    //    }
    //    else
    //    {
    //        m_problemInfo->setModule(moduleFactory(cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString().toStdString(),
    //                                               (ProblemType) cmbProblemType->itemData(cmbProblemType->currentIndex()).toInt(),
    //                                               (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt(),
    //                                               (cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString() == "custom"
    //                                                ? Util::scene()->problemInfo()->fileName.left(Util::scene()->problemInfo()->fileName.size() - 4) + ".xml" : "").toStdString()));
    //    }

            // check values
            //TODO
            //    if (cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt() == AnalysisType_Harmonic)
            //    {
            //        if (txtFrequency->value() < 0)
            //        {
            //            QMessageBox::critical(this, tr("Error"), tr("Frequency cannot be negative."));
            //            return false;
            //        }
            //    }
            //    if (cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt() == AnalysisType_Transient)
            //    {
            //        txtTransientTimeStep->evaluate(false);
            //        if (txtTransientTimeStep->number() <= 0.0)
            //        {
            //            QMessageBox::critical(this, tr("Error"), tr("Time step must be positive."));
            //            return false;
            //        }
            //        txtTransientTimeTotal->evaluate(false);
            //        if (txtTransientTimeTotal->number() <= 0.0)
            //        {
            //            QMessageBox::critical(this, tr("Error"), tr("Total time must be positive."));
            //            return false;
            //        }
            //        txtTransientTimeStep->evaluate(false);
            //        if (txtTransientTimeStep->number() > txtTransientTimeTotal->number())
            //        {
            //            QMessageBox::critical(this, tr("Error"), tr("Time step is greater then total time."));
            //            return false;
            //        }
            //    }

            // run and check startup script
            if (!txtStartupScript->toPlainText().isEmpty())
    {
        ScriptResult scriptResult = runPythonScript(txtStartupScript->toPlainText());
        if (scriptResult.isError)
        {
            QMessageBox::critical(QApplication::activeWindow(), QObject::tr("Error"), scriptResult.text);
            return false;
        }
    }

    // save properties
    m_problemInfo->name = txtName->text();
    m_problemInfo->date = dtmDate->date();
    m_problemInfo->coordinateType = (CoordinateType) cmbCoordinateType->itemData(cmbCoordinateType->currentIndex()).toInt();

    m_problemInfo->frequency = txtFrequency->value();

    m_problemInfo->timeStep = txtTransientTimeStep->value();
    m_problemInfo->timeTotal = txtTransientTimeTotal->value();

    m_problemInfo->description = txtDescription->toPlainText();
    m_problemInfo->scriptStartup = txtStartupScript->toPlainText();

    // matrix solver
    m_problemInfo->matrixSolver = (Hermes::MatrixSolverType) cmbMatrixSolver->itemData(cmbMatrixSolver->currentIndex()).toInt();

    // save fields
    for (int i = 0; i < tabFields->count(); i++)
        static_cast<FieldWidget *>(tabFields->widget(i))->save();

    return true;
}

void ProblemDialog::doAccept()
{
    logMessage("ProblemDialog::doAccept()");

    if (save()) accept();
}

void ProblemDialog::doReject()
{
    logMessage("ProblemDialog::doReject()");

    reject();
}

void ProblemDialog::doOpenXML()
{
    logMessage("ProblemDialog::doOpenXML()");

    QString fileName;
    //TODO custom
    //    if (cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString() == "custom")
    //    {
    //        fileName = Util::scene()->problemInfo()->fileName.left(Util::scene()->problemInfo()->fileName.size() - 4) + ".xml";

    //        if (!QFile::exists(fileName))
    //            if (QMessageBox::question(this, tr("Custom module file"), tr("Custom module doesn't exist. Could I create it?"),
    //                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    //            {
    //                // copy custom module
    //                QFile::copy(datadir() + "/resources/custom.xml",
    //                            fileName);
    //            }
    //    }
    //    else
    //    {
    //        fileName = datadir() + "/modules/" + cmbPhysicField->itemData(cmbPhysicField->currentIndex()).toString() + ".xml";
    //    }

    //    if (QFile::exists(fileName))
    //        QDesktopServices::openUrl(QUrl(fileName));
}

void ProblemDialog::doPhysicFieldChanged(int index)
{
    logMessage("ProblemDialog::doPhysicFieldChanged()");

    // refresh modules
    for (int i = 0; i < tabFields->count(); i++)
    {
        FieldWidget *wid = dynamic_cast<FieldWidget *>(tabFields->widget(i));
        wid->refresh();
    }
}

void ProblemDialog::doTransientChanged()
{
    logMessage("ProblemDialog::doTransientChanged()");

    if (txtTransientTimeStep->evaluate(true) &&
            txtTransientTimeTotal->evaluate(true))
    {
        lblTransientSteps->setText(QString("%1").arg(floor(txtTransientTimeTotal->number()/txtTransientTimeStep->number())));
    }
}

void ProblemDialog::doAddField()
{
    FieldSelectDialog dialog(this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        Util::scene()->addField(new FieldInfo(m_problemInfo, dialog.fieldId()));

        refresh();
    }
}

