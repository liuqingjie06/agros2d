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

#ifndef SCENEBASIC_H
#define SCENEBASIC_H

#include "util.h"

class SLineEditDouble;
class ValueLineEdit;

struct Point;

class SceneBasic;
template <typename MarkerType> class MarkedSceneBasic;
template <typename MarkerType> class UniqueMarkerContainer;
class SceneNode;
class SceneEdge;
class SceneLabel;
class Marker;

class SceneBoundary;
class SceneMaterial;

Q_DECLARE_METATYPE(SceneBasic *)
Q_DECLARE_METATYPE(SceneNode *)
Q_DECLARE_METATYPE(SceneEdge *)
Q_DECLARE_METATYPE(SceneLabel *)

class SceneBasic 
{

public:
    bool isSelected;
    bool isHighlighted;

    SceneBasic();
    void setSelected(bool value = true) { isSelected = value; }

    virtual int showDialog(QWidget *parent, bool isNew = false) = 0;

    QVariant variant();
};

template <typename BasicType>
class SceneBasicContainer
{
public:
    /// items() should be removed step by step from the code.
    /// more methods operating with list data should be defined here
    QList<BasicType*> items() { return data; }

    bool add(BasicType *item);
    bool remove(BasicType *item);
    BasicType *at(int i);
    inline int length() { return data.length(); }
    inline int isEmpty() { return data.isEmpty(); }
    void clear();

    /// selects or unselects all items
    void setSelected(bool value = true);

protected:
    QList<BasicType*> data;
};

// *************************************************************************************************************************************

class SceneNode : public SceneBasic 
{
public:
    Point point;

    SceneNode(const Point &point);

    double distance(const Point &point) const;

    int showDialog(QWidget *parent, bool isNew = false);
};

class SceneNodeContainer : public SceneBasicContainer<SceneNode>
{

};

// *************************************************************************************************************************************

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO zamyslet se nad porovnavanim markeru
//TODO opravdu chci porovnavat ukazatele?
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


template <typename MarkerType>
class MarkedSceneBasic : public SceneBasic
{
public:
    /// gets marker that corresponds to the given field
    MarkerType* getMarker(QString field);

    /// true if has given marker
    bool hasMarker(MarkerType* marker) {return markers->contains(marker); }

public:
    UniqueMarkerContainer<MarkerType> *markers;
};


template <typename MarkerType, typename MarkedSceneBasicType>
class MarkedSceneBasicContainer : public SceneBasicContainer<MarkedSceneBasicType>
{
public:
    UniqueMarkerContainer<MarkerType> allMarkers();
    void removeMarkerFromAll(MarkerType* marker);
    void addMarkerToAll(MarkerType* marker);

    /// Filters for elements that has given marker
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> haveMarker(MarkerType *marker);

    //TODO unfortunately, those had to be moved here from SceneBasicContainer
    //TODO if they returned SceneBasicContainer, One would have to cast to use methods of this class to return value...
    //TODO it might be possible to do it differently...
    /// Filters for selected
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> selected();

    /// Filters for highlighted
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> highlited();
};

class SceneEdge : public MarkedSceneBasic<SceneBoundary>
{
public:
    SceneNode *nodeStart;
    SceneNode *nodeEnd;
    double angle;
    int refineTowardsEdge;

    SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, double angle, int refineTowardsEdge);

    Point center() const;
    double radius() const;
    double distance(const Point &point) const;
    int segments() const; // needed by mesh generator
    double length() const;
    bool isStraight() const { return (fabs(angle) < EPS_ZERO); }

    int showDialog(QWidget *parent, bool isNew = false);
};

class SceneEdgeContainer : public MarkedSceneBasicContainer<SceneBoundary, SceneEdge>
{
public:
    void removeConnectedToNode(SceneNode* node);
};

// *************************************************************************************************************************************

class SceneLabel : public MarkedSceneBasic<SceneMaterial>
{
public:
    Point point;
    double area;
    int polynomialOrder;

    SceneLabel(const Point &point, double area, int polynomialOrder);

    double distance(const Point &point) const;

    int showDialog(QWidget *parent, bool isNew = false);
};

class SceneLabelContainer : public MarkedSceneBasicContainer<SceneMaterial, SceneLabel>
{

};



// *************************************************************************************************************************************
// *************************************************************************************************************************************

class DSceneBasic: public QDialog
{
    Q_OBJECT

public:
    DSceneBasic(QWidget *parent, bool isNew = false);
    ~DSceneBasic();

protected:
    bool isNew;
    SceneBasic *m_object;
    QDialogButtonBox *buttonBox;

    virtual QLayout *createContent() = 0;
    void createControls();

    virtual bool load() = 0;
    virtual bool save() = 0;

protected slots:
    void evaluated(bool isError);

private:    
    QVBoxLayout *layout;

private slots:
    void doAccept();
    void doReject();
};

// *************************************************************************************************************************************

class DSceneNode : public DSceneBasic
{
    Q_OBJECT

public:
    DSceneNode(SceneNode *node, QWidget *parent, bool isNew = false);
    ~DSceneNode();

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;
    QLabel *lblDistance;
    QLabel *lblAngle;

private slots:
    void doEditingFinished();
};

// *************************************************************************************************************************************

class SceneEdgeDialog : public DSceneBasic
{
    Q_OBJECT

public:
    SceneEdgeDialog(SceneEdge *edge, QWidget *parent, bool isNew);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    QLabel *lblEquation;
    QComboBox *cmbNodeStart;
    QComboBox *cmbNodeEnd;
    QComboBox *cmbBoundary;
    QPushButton *btnBoundary;
    ValueLineEdit *txtAngle;
    QLabel *lblLength;
    QCheckBox *chkRefineTowardsEdge;
    QSpinBox *txtRefineTowardsEdge;

    void fillComboBox();

private slots:
    void doBoundaryChanged(int index);
    void doBoundaryClicked();
    void doNodeChanged();
    void doRefineTowardsEdge(int state);
};

// *************************************************************************************************************************************

class SceneLabelDialog : public DSceneBasic
{
    Q_OBJECT

public:
    SceneLabelDialog(SceneLabel *label, QWidget *parent, bool isNew = false);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;
    QComboBox *cmbMaterial;
    QPushButton *btnMaterial;
    ValueLineEdit *txtArea;
    QSpinBox *txtPolynomialOrder;
    QCheckBox *chkArea;
    QCheckBox *chkPolynomialOrder;

    void fillComboBox();

private slots:
    void doMaterialChanged(int index);
    void doMaterialClicked();
    void doArea(int);
    void doPolynomialOrder(int);
};

// undo framework *******************************************************************************************************************

// Node

class SceneNodeCommandAdd : public QUndoCommand
{
public:
    SceneNodeCommandAdd(const Point &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
};

class SceneNodeCommandRemove : public QUndoCommand
{
public:
    SceneNodeCommandRemove(const Point &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
};

class SceneNodeCommandEdit : public QUndoCommand
{
public:
    SceneNodeCommandEdit(const Point &point, const Point &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    Point m_pointNew;
};

// Label

class SceneLabelCommandAdd : public QUndoCommand
{
public:
    SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
    int m_polynomialOrder;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    QString m_markerName;
    double m_area;
    int m_polynomialOrder;
};

class SceneLabelCommandEdit : public QUndoCommand
{
public:
    SceneLabelCommandEdit(const Point &point, const Point &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_point;
    Point m_pointNew;
};

// Edge

class SceneEdgeCommandAdd : public QUndoCommand
{
public:
    SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                        double angle, int refineTowardsEdge, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QString m_markerName;
    double m_angle;
    int m_refineTowardsEdge;
};

class SceneEdgeCommandRemove : public QUndoCommand
{
public:
    SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                           double angle, int refineTowardsEdge, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    QString m_markerName;
    double m_angle;
    int m_refineTowardsEdge;
};

class SceneEdgeCommandEdit : public QUndoCommand
{
public:
    SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                         double angle, double angleNew, int refineTowardsEdge, int refineTowardsEdgeNew, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    Point m_pointStart;
    Point m_pointEnd;
    Point m_pointStartNew;
    Point m_pointEndNew;
    double m_angle;
    double m_angleNew;
    int m_refineTowardsEdge;
    int m_refineTowardsEdgeNew;
};

#endif // SCENEBASIC_H
