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

#ifndef SCENEVIEWMESH_H
#define SCENEVIEWMESH_H

#include "util.h"
#include "sceneview_common2d.h"


class MeshHermes : public QObject
{
    Q_OBJECT

public:
    MeshHermes();
    ~MeshHermes();

    void clear();

    // mesh
    inline Hermes::Hermes2D::Views::Linearizer &linInitialMeshView() { return m_linInitialMeshView; }
    inline Hermes::Hermes2D::Views::Linearizer &linSolutionMeshView() { return m_linSolutionMeshView; }

    // order view
    Hermes::Hermes2D::Views::Orderizer &ordView() { return m_orderView; }

public slots:
    virtual void meshed();
    virtual void solved();

private:
    // initial mesh
    Hermes::Hermes2D::Views::Linearizer m_linInitialMeshView;

    // solution mesh
    Hermes::Hermes2D::Views::Linearizer m_linSolutionMeshView;

    // order view
    Hermes::Hermes2D::Views::Orderizer m_orderView;

    // process
    void processOrder();
    void processInitialMesh();
    void processSolutionMesh();
};

class SceneViewMesh : public SceneViewCommon2D
{
    Q_OBJECT

public:
    SceneViewMesh(QWidget *parent = 0);
    ~SceneViewMesh();

    QAction *actSceneModeMesh;

public slots:
    virtual void doInvalidated();
    virtual void clear();

protected:
    virtual void paintGL();

    void paintGeometry();

    void paintInitialMesh();
    void paintSolutionMesh();
    void paintOrder();
    void paintOrderColorBar();

protected slots:

private:
    // gl lists
    int m_listInitialMesh;
    int m_listSolutionMesh;
    int m_listOrder;

    MeshHermes *m_meshHermes;

    void createActionsMesh();
};

#endif // SCENEVIEWMESH_H