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

#ifndef SCENEVIEWDIALOG_H
#define SCENEVIEWDIALOG_H

#include "util.h"

class SceneViewGeometry;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class SLineEditDouble;

class PostprocessorView : public QDockWidget
{
    Q_OBJECT

public:
    PostprocessorView(SceneViewGeometry *sceneGeometry,
                      SceneViewMesh *sceneMesh,
                      SceneViewPost2D *scenePost2D,
                      SceneViewPost3D *scenePost3D,
                      QWidget *parent);

private slots:
    void doApply();

private:
    SceneViewGeometry *m_sceneGeometry;
    SceneViewMesh *m_sceneMesh;
    SceneViewPost2D *m_scenePost2D;
    SceneViewPost3D *m_scenePost3D;

    QWidget *basic;
    QWidget *postprocessor;
    QWidget *workspace;

    // basic
    QComboBox *cmbFieldInfo;

    QGroupBox *grpShowMesh;
    QGroupBox *grpShowPost2D;
    QGroupBox *grpShowPost3D;

    // show
    QCheckBox *chkShowInitialMeshView;
    QCheckBox *chkShowSolutionMeshView;
    QCheckBox *chkShowPost2DContourView;
    QCheckBox *chkShowPost2DScalarView;
    QCheckBox *chkShowOrderView;
    QCheckBox *chkShowPost2DVectorView;
    QCheckBox *chkShowPost2DParticleView;

    QButtonGroup *butPost3DGroup;
    QRadioButton *radPost3DNone;
    QRadioButton *radPost3DScalarField3D;
    QRadioButton *radPost3DScalarField3DSolid;
    QRadioButton *radPost3DModel;
    QRadioButton *radPost3DParticleTracing;

    // scalar field
    QGroupBox *grpPost2DScalarField;
    QComboBox *cmbPost2DScalarFieldVariable;
    QComboBox *cmbPost2DScalarFieldVariableComp;
    QLineEdit *txtPost2DScalarFieldExpression;
    QCheckBox *chkScalarFieldRangeAuto;
    QLabel *lblScalarFieldRangeMin;
    QLabel *lblScalarFieldRangeMax;
    SLineEditDouble *txtScalarFieldRangeMin;
    SLineEditDouble *txtScalarFieldRangeMax;
    QLabel *lblScalarFieldRangeMinError;
    QLabel *lblScalarFieldRangeMaxError;

    // vector field
    QGroupBox *grpPost2DVectorField;
    QComboBox *cmbPost2DVectorFieldVariable;

    // transient
    QGroupBox *grpTransient;
    QComboBox *cmbTimeStep;

    // adaptivity
    QGroupBox *grpAdaptivity;
    QComboBox *cmbAdaptivityStep;
    QComboBox *cmbAdaptivitySolutionType;

    // grid
    QLineEdit *txtGridStep;
    QCheckBox *chkShowGrid;
    QCheckBox *chkSnapToGrid;

    // general view
    QCheckBox *chkZoomToMouse;
    QSpinBox *txtGeometryNodeSize;
    QSpinBox *txtGeometryEdgeWidth;
    QSpinBox *txtGeometryLabelSize;

    // scene font
    QLabel *lblSceneFontExample;
    QPushButton *btnSceneFont;

    // workspace other
    QCheckBox *chkShowAxes;
    QCheckBox *chkShowRulers;

    // scalar field
    QCheckBox *chkShowScalarColorBar;
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QSpinBox *txtPaletteSteps;
    QComboBox *cmbLinearizerQuality;
    QCheckBox *chkScalarFieldRangeLog;
    SLineEditDouble *txtScalarFieldRangeBase;
    QSpinBox *txtScalarDecimalPlace;

    // contours
    QGroupBox *grpPost2DContourField;
    QComboBox *cmbPost2DContourVariable;
    QSpinBox *txtContoursCount;

    // vector field
    QCheckBox *chkVectorProportional;
    QCheckBox *chkVectorColor;
    QSpinBox *txtVectorCount;
    QDoubleSpinBox *txtVectorScale;

    // polynomial order
    QCheckBox *chkShowOrderScale;
    QComboBox *cmbOrderPaletteOrder;
    QCheckBox *chkOrderLabel;

    // particle tracing
    QCheckBox *chkParticleIncludeGravitation;
    QSpinBox *txtParticleNumberOfParticles;
    SLineEditDouble *txtParticleStartingRadius;
    SLineEditDouble *txtParticleMass;
    SLineEditDouble *txtParticleConstant;
    SLineEditDouble *txtParticlePointX;
    SLineEditDouble *txtParticlePointY;
    SLineEditDouble *txtParticleVelocityX;
    SLineEditDouble *txtParticleVelocityY;
    SLineEditDouble *txtParticleMaximumRelativeError;
    QCheckBox *chkParticleTerminateOnDifferentMaterial;
    QLabel *lblParticlePointX;
    QLabel *lblParticlePointY;
    QLabel *lblParticleVelocityX;
    QLabel *lblParticleVelocityY;
    QLabel *lblParticleMotionEquations;
    QCheckBox *chkParticleColorByVelocity;
    QCheckBox *chkParticleShowPoints;
    QSpinBox *txtParticleMaximumSteps;
    SLineEditDouble *txtParticleDragDensity;
    SLineEditDouble *txtParticleDragCoefficient;
    SLineEditDouble *txtParticleDragReferenceArea;

    // advanced
    QCheckBox *chkView3DLighting;
    QDoubleSpinBox *txtView3DAngle;
    QCheckBox *chkView3DBackground;
    QDoubleSpinBox *txtView3DHeight;
    QCheckBox *chkDeformScalar;
    QCheckBox *chkDeformContour;
    QCheckBox *chkDeformVector;

    QToolBox *tbxWorkspace;
    QToolBox *tbxPostprocessor;
    QPushButton *btnOK;

    void loadBasic();
    void loadAdvanced();
    void saveBasic();
    void saveAdvanced();

    void createControls();
    QWidget *controlsBasic();
    QWidget *controlsPostprocessor();
    QWidget *controlsWorkspace();

signals:
    void apply();

public slots:
    void updateControls();
    void setControls();

private slots:
    void doFieldInfo(int index);
    void doScalarFieldVariable(int index);
    void doScalarFieldVariableComp(int index);
    void doScalarFieldRangeAuto(int state);
    void doPostprocessorGroupClicked(QAbstractButton *button);
    void doScalarFieldRangeMinChanged();
    void doScalarFieldRangeMaxChanged();
    void doPaletteFilter(int state);
    void doWorkspaceDefault();
    void doScalarFieldDefault();
    void doContoursDefault();
    void doVectorFieldDefault();
    void doAdvancedDefault();
    void doParticleDefault();
    void doOrderDefault();
    void doSceneFont();
    void doShowGridChanged();
    void doScalarFieldLog(int state);
};

#endif // SCENEVIEWDIALOG_H
