// SPDX-FileCopyrightText: 2011-2019 Disney Enterprises, Inc.
// SPDX-License-Identifier: LicenseRef-Apache-2.0
// SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * @file ExprCurve.cpp
 * @brief Contains PyQt4 Ramp Widget to emulate Maya's ramp widget
 * @author Arthur Shek
 * @version ashek     05/04/09  Initial Version
 */

#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QToolButton>
#include <QVBoxLayout>

#include <KSeExpr/ExprBuiltins.h>

#include "ExprCurve.h"

void CurveScene::removeAll()
{
    _cvs.clear();
}

void CurveGraphicsView::resizeEvent(QResizeEvent *event)
{
    emit resizeSignal(event->size().width(), event->size().height());
}

CurveScene::CurveScene()
    : _curve(new T_CURVE)
    , _width(320)
    , _height(50)
    , _interp(T_CURVE::kMonotoneSpline)
    , _selectedItem(-1)
    , _lmb(false)
{
    rebuildCurve();
    resize(_width, _height);
}

CurveScene::~CurveScene()
{
    delete _curve;
}

void CurveScene::resize(const int width, const int height)
{
    // width and height already have the 8 px padding factored in
    _width = width - 16;
    _height = height - 16;
    setSceneRect(-9, -7, width, height);
    drawRect();
    drawPoly();
    drawPoints();
}

void CurveScene::rebuildCurve()
{
    delete _curve;
    _curve = new T_CURVE;
    for (auto & _cv : _cvs)
        _curve->addPoint(_cv._pos, _cv._val, _cv._interp);
    _curve->preparePoints();
}

void CurveScene::addPoint(double x, double y, const T_INTERP interp, const bool select)
{
    x = KSeExpr::clamp(x, 0, 1);
    y = KSeExpr::clamp(y, 0, 1);

    _cvs.emplace_back(x, y, T_INTERP(interp));
    auto newIndex = _cvs.size() - 1;

    rebuildCurve();

    if (select)
        _selectedItem = newIndex;
    drawPoly();
    drawPoints();
}

void CurveScene::removePoint(const int index)
{
    _cvs.erase(_cvs.begin() + index);
    _selectedItem = -1;
    rebuildCurve();

    drawPoly();
    drawPoints();
    emitCurveChanged();
}

void CurveScene::keyPressEvent(QKeyEvent *event)
{
    if (((event->key() == Qt::Key_Backspace) || (event->key() == Qt::Key_Delete)) && (_selectedItem >= 0)) {
        // user hit delete with cv selected
        removePoint(_selectedItem);
    }
}

void CurveScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    _lmb = true;
    QPointF pos = mouseEvent->scenePos();
    // get items under mouse click
    QList<QGraphicsItem *> itemList = items(pos);
    if (itemList.empty()) {
        _selectedItem = -1;
        emit cvSelected(-1, -1, _interp);
        drawPoints();
    } else if (itemList[0]->zValue() == 2) {
        // getting here means we've selected a current point
        const int numCircle = _circleObjects.size();
        for (int i = 0; i < numCircle; i++) {
            QGraphicsItem *obj = _circleObjects[i];
            if (obj == itemList[0]) {
                _selectedItem = i;
                _interp = _cvs[i]._interp;
                emit cvSelected(_cvs[i]._pos, _cvs[i]._val, _cvs[i]._interp);
            }
        }
        drawPoints();
    } else {
        if (mouseEvent->buttons() == Qt::LeftButton) {
            // getting here means we want to create a new point
            double myx = pos.x() / _width;
            T_INTERP interpFromNearby = _curve->getLowerBoundCV(KSeExpr::clamp(myx, 0, 1))._interp;
            if (interpFromNearby == T_CURVE::kNone)
                interpFromNearby = T_CURVE::kMonotoneSpline;
            addPoint(myx, pos.y() / _height, interpFromNearby);
            emitCurveChanged();
        } else {
            _selectedItem = -1;
            drawPoints();
        }
    }
}

void CurveScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (_selectedItem >= 0) {
        auto *menu = new QMenu(event->widget());
        QAction *deleteAction = menu->addAction(tr("Delete Point"));
        // menu->addAction("Cancel");
        QAction *action = menu->exec(event->screenPos());
        if (action == deleteAction)
            removePoint(_selectedItem);
    }
}

void CurveScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (_lmb) {
        QPointF point = mouseEvent->scenePos();
        if (_selectedItem >= 0) {
            // clamp motion to inside curve area
            double pos = KSeExpr::clamp(point.x() / _width, 0, 1);
            double val = KSeExpr::clamp(point.y() / _height, 0, 1);
            _cvs[_selectedItem]._pos = pos;
            _cvs[_selectedItem]._val = val;
            rebuildCurve();
            emit cvSelected(pos, val, _cvs[_selectedItem]._interp);
            drawPoly();
            drawPoints();
            emitCurveChanged();
        }
    }
}

void CurveScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    _lmb = false;
}

// user selected a different interpolation type, redraw
void CurveScene::interpChanged(const int interp)
{
    _interp = T_INTERP(interp);
    if (_selectedItem >= 0) {
        _cvs[_selectedItem]._interp = _interp;
        rebuildCurve();
        drawPoly();
        emitCurveChanged();
    }
}

// user entered a different point position, redraw
void CurveScene::selPosChanged(double posInput)
{
    if (_selectedItem >= 0) {
        double pos = KSeExpr::clamp(posInput, 0, 1);
        _cvs[_selectedItem]._pos = pos;
        rebuildCurve();
        drawPoly();
        drawPoints();
        emitCurveChanged();
    }
}

// user entered a different point value, redraw
void CurveScene::selValChanged(double val)
{
    if (_selectedItem >= 0) {
        val = KSeExpr::clamp(val, 0, 1);
        _cvs[_selectedItem]._val = val;
        rebuildCurve();
        drawPoly();
        drawPoints();
        emitCurveChanged();
    }
}

// return points in reverse order in order to use same parsing in editor
void CurveScene::emitCurveChanged()
{
    emit curveChanged();
}

// draws the base gray outline rectangle
void CurveScene::drawRect()
{
    if (_baseRect == nullptr) {
        _baseRect = addRect(0, 0, _width, _height, QPen(Qt::black, 1.0), QBrush(Qt::gray));
    }
    _baseRect->setRect(0, 0, _width, _height);
    _baseRect->setZValue(0);
}

// draws the poly curve representation
void CurveScene::drawPoly()
{
    if (_curvePoly == nullptr) {
        _curvePoly = addPolygon(QPolygonF(), QPen(Qt::black, 1.0), QBrush(Qt::darkGray));
    }

    QPolygonF poly;
    poly.append(QPointF(_width, 0));
    poly.append(QPointF(0, 0));
    for (int i = 0; i < 1000; i++) {
        double x = i / 1000.0;
        poly.append(QPointF(_width * x, _height * _curve->getValue(x)));
    }
    poly.append(QPointF(_width, 0));
    _curvePoly->setPolygon(poly);
    _curvePoly->setZValue(1);
}

// draws the cv points
void CurveScene::drawPoints()
{
    for(const auto *i: _circleObjects) {
        delete i;
    }
    _circleObjects.clear();
    int numCV = _cvs.size();
    for (int i = 0; i < numCV; i++) {
        const T_CURVE::CV &pt = _cvs[i];
        QPen pen;
        if (i == _selectedItem) {
            pen = QPen(Qt::white, 1.0);
        } else {
            pen = QPen(Qt::black, 1.0);
        }
        _circleObjects.push_back(addEllipse(pt._pos * _width - 4, pt._val * _height - 4, 8, 8, pen, QBrush()));
        QGraphicsEllipseItem *circle = _circleObjects.back();
        circle->setFlag(QGraphicsItem::ItemIsMovable, true);
        circle->setZValue(2);
    }
}

ExprCurve::ExprCurve(QWidget *parent, QString pLabel, QString vLabel, QString, bool expandable)
    : QWidget(parent)
    , _scene(nullptr)
{
    auto *mainLayout = new QHBoxLayout();
    mainLayout->setMargin(0);

    auto *edits = new QWidget;
    auto *editsLayout = new QFormLayout;
    editsLayout->setMargin(0);
    edits->setLayout(editsLayout);

    _selPosEdit = new QLineEdit;
    auto *posValidator = new QDoubleValidator(0.0, 1.0, 6, _selPosEdit);
    _selPosEdit->setValidator(posValidator);
    QString posLabel;
    if (pLabel.isEmpty()) {
        posLabel = tr("Selected Position:");
    } else {
        posLabel = pLabel;
    }
    editsLayout->addRow(posLabel, _selPosEdit);

    _selValEdit = new QLineEdit;
    auto *valValidator = new QDoubleValidator(0.0, 1.0, 6, _selValEdit);
    _selValEdit->setValidator(valValidator);
    QString valLabel;
    if (vLabel.isEmpty()) {
        valLabel = tr("Selected Value:");
    } else {
        valLabel = vLabel;
    }
    editsLayout->addRow(valLabel, _selValEdit);

    _interpComboBox = new QComboBox;
    _interpComboBox->addItem(tr("None"));
    _interpComboBox->addItem(tr("Linear"));
    _interpComboBox->addItem(tr("Smooth"));
    _interpComboBox->addItem(tr("Spline"));
    _interpComboBox->addItem(tr("MSpline"));
    _interpComboBox->setCurrentIndex(4);

    editsLayout->addWidget(_interpComboBox);

    auto *curveView = new CurveGraphicsView;
    curveView->setFrameShape(QFrame::StyledPanel);
    curveView->setFrameShadow(QFrame::Sunken);
    curveView->setLineWidth(1);
    curveView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    curveView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scene = new CurveScene;
    curveView->setScene(_scene);
    curveView->setTransform(QTransform().scale(1, -1));
    curveView->setRenderHints(QPainter::Antialiasing);

    mainLayout->addWidget(edits);
    mainLayout->addWidget(curveView);
    if (expandable) {
        auto *expandButton = new QToolButton(this);
        expandButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        QIcon expandIcon = QIcon::fromTheme("arrow-right", QIcon::fromTheme("go-next"));
        auto *detailAction = new QAction(expandIcon, tr("&Expand..."));
        expandButton->setDefaultAction(detailAction);
        mainLayout->addWidget(expandButton);
        // open a the detail widget when clicked
        connect(expandButton, SIGNAL(triggered(QAction *)), this, SLOT(openDetail()));
    }
    mainLayout->setStretchFactor(curveView, 100);
    setLayout(mainLayout);

    // SIGNALS

    // when a user selects a cv, update the fields on left
    connect(_scene, SIGNAL(cvSelected(double, double, T_INTERP)), this, SLOT(cvSelectedSlot(double, double, T_INTERP)));
    // when a user selects a different interp, the curve has to redraw
    connect(_interpComboBox, SIGNAL(activated(int)), _scene, SLOT(interpChanged(int)));
    // when a user types a different position, the curve has to redraw
    connect(_selPosEdit, SIGNAL(returnPressed()), this, SLOT(selPosChanged()));
    connect(this, SIGNAL(selPosChangedSignal(double)), _scene, SLOT(selPosChanged(double)));
    // when a user types a different value, the curve has to redraw
    connect(_selValEdit, SIGNAL(returnPressed()), this, SLOT(selValChanged()));
    connect(this, SIGNAL(selValChangedSignal(double)), _scene, SLOT(selValChanged(double)));
    // when the widget is resized, resize the curve widget
    connect(curveView, SIGNAL(resizeSignal(int, int)), _scene, SLOT(resize(int, int)));
}

// CV selected, update the user interface fields.
void ExprCurve::cvSelectedSlot(double pos, double val, T_INTERP interp)
{
    QString posStr;
    if (pos >= 0.0)
        posStr.setNum(pos, 'f', 3);
    _selPosEdit->setText(posStr);
    QString valStr;
    if (val >= 0.0)
        valStr.setNum(val, 'f', 3);
    _selValEdit->setText(valStr);
    _interpComboBox->setCurrentIndex(interp);
}

// User entered new position, round and send signal to redraw curve.
void ExprCurve::selPosChanged()
{
    double pos = QString(_selPosEdit->text()).toDouble();
    _selPosEdit->setText(QString(tr("%1")).arg(pos, 0, 'f', 3));
    emit selPosChangedSignal(pos);
}

// User entered new value, round and send signal to redraw curve.
void ExprCurve::selValChanged()
{
    double val = QString(_selValEdit->text()).toDouble();
    val = KSeExpr::clamp(val, 0, 1);
    _selValEdit->setText(QString(tr("%1")).arg(val, 0, 'f', 3));
    emit selValChangedSignal(val);
}

void ExprCurve::openDetail()
{
    auto *dialog = new QDialog();
    dialog->setMinimumWidth(1024);
    dialog->setMinimumHeight(400);
    auto *curve = new ExprCurve(nullptr, QString(), QString(), QString(), false);

    // copy points into new data
    const std::vector<T_CURVE::CV> &data = _scene->_cvs;
    for (const auto & i : data)
        curve->addPoint(i._pos, i._val, i._interp);

    auto *layout = new QVBoxLayout();
    dialog->setLayout(layout);
    layout->addWidget(curve);
    auto *buttonbar = new QDialogButtonBox();
    buttonbar->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonbar, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonbar, SIGNAL(rejected()), dialog, SLOT(reject()));
    layout->addWidget(buttonbar);

    if (dialog->exec() == QDialog::Accepted) {
        // copy points back from child
        _scene->removeAll();
        const auto &dataNew = curve->_scene->_cvs;
        for (const auto & i : dataNew)
            addPoint(i._pos, i._val, i._interp);
        _scene->emitCurveChanged();
    }

    if (dialog->exec() == QDialog::Accepted) {
        // copy points back from child
        _scene->removeAll();
        const auto &dataNew = curve->_scene->_cvs;
        for (const auto & i : dataNew)
            addPoint(i._pos, i._val, i._interp);
        _scene->emitCurveChanged();
    }
}
