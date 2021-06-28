// SPDX-FileCopyrightText: 2011-2019 Disney Enterprises, Inc.
// SPDX-License-Identifier: LicenseRef-Apache-2.0
// SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * @file ExprControl.cpp
 * @brief UI control widgets for expressions.
 * @author  aselle
 */

#include "ExprControl.h"
#include "Debug.h"
#include "Editable.h"
#include "ExprColorCurve.h"
#include "ExprColorSwatch.h"
#include "ExprFileDialog.h"


/* XPM */
static constexpr std::array<const char *, 24> directoryXPM = {"20 20 3 1",
                                                              ". c None",
                                                              "# c #000000",
                                                              "a c #d8c59e",
                                                              "....................",
                                                              "....................",
                                                              "....................",
                                                              "....................",
                                                              "...........#######..",
                                                              "...........#aaaaa#..",
                                                              "..##########aaaaa#..",
                                                              "..#aaaaaaaaaaaaaa#..",
                                                              "..#aaaaaaaaaaaaaa#..",
                                                              "..#aaaaaaaaaaaaaa#..",
                                                              "..#aaaaaaaaaaaaaa#..",
                                                              "..#aaaaaaaaaaaaaa#..",
                                                              "..#aaaaa##a##a##a#..",
                                                              "..#aaaaa##a##a##a#..",
                                                              "..#aaaaaaaaaaaaaa#..",
                                                              "..################..",
                                                              "....................",
                                                              "....................",
                                                              "....................",
                                                              "...................."};

/* XPM */
static constexpr std::array<const char *, 26> fileXPM = {"20 20 5 1",
                                                         ". c None",
                                                         "# c #000000",
                                                         "c c #303030",
                                                         "b c #a79b80",
                                                         "a c #ddcdaa",
                                                         "....................",
                                                         "....................",
                                                         "....#########.......",
                                                         "....#aaaaaaa##......",
                                                         "....#aaaaaaa#b#.....",
                                                         "....#aaaaaaa#bb#....",
                                                         "....#aaaaaaa####....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....#accaccacca#....",
                                                         "....#accaccacca#....",
                                                         "....#aaaaaaaaaa#....",
                                                         "....############....",
                                                         "...................."};

void ExprSlider::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void ExprSlider::mouseMoveEvent(QMouseEvent *e)
{
    auto r = maximum() - minimum();
    auto v = ((double)(e->x() - 2) * r / (width() - 5)) + minimum() + .5F;
    int iv = std::min(std::max((int)v, minimum()), maximum());
    setValue(iv);
}

void ExprSlider::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    double v = value();
    double r = maximum() - minimum();
    int linepos = (int)((v - minimum()) / r * (width() - 5) + 2);

    QColor qcol = palette().color(QPalette::Dark);
    QColor bcol = palette().color(QPalette::Midlight);
    QColor dcol = bcol.lighter(140);
    QColor bgcol = palette().color(QPalette::Base);

    if (underMouse()) {
        bcol = bcol.lighter(110);
        bgcol = bgcol.lighter(110);
        int mx = mapFromGlobal(QCursor::pos()).x();
        if (abs(linepos - mx) < 4)
            dcol = dcol.lighter(200);
    }

    p.fillRect(1, 1, width() - 1, height() - 2, bgcol);
    p.fillRect(1, 1, linepos - 1, height() - 2, bcol);

    QPen pen = p.pen();

    pen.setColor(dcol);
    p.setPen(pen);
    pen.setWidth(3);
    p.setPen(pen);
    p.drawLine(linepos, 2, linepos, height() - 2);
    pen.setWidth(1);
    pen.setColor(qcol);
    p.setPen(pen);
    p.drawLine(linepos - 2, 1, linepos - 2, height() - 1);
    p.drawLine(linepos + 2, 1, linepos + 2, height() - 1);

    pen.setWidth(1);
    pen.setColor(qcol);
    p.setPen(pen);
    p.drawRect(0, 0, width() - 1, height() - 1);
}

ExprChannelSlider::ExprChannelSlider(int id, QWidget *parent)
    : QWidget(parent)
    , _id(id)
    , _value(0)
{
}

void ExprChannelSlider::paintEvent(QPaintEvent *)
{
    if (_value < 0 || _value > 1)
        return;
    int x = int(_value * (width() - 3) + 0.5);
    QPainter p(this);
    p.fillRect(contentsRect(), _col);
    p.fillRect(x, 0, 3, height(), QColor(64, 64, 64));
}

void ExprChannelSlider::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void ExprChannelSlider::mouseMoveEvent(QMouseEvent *e)
{
    setValue(clamp(float(e->x() - 1) / (width() - 3), 0, 1));
}

void ExprChannelSlider::setValue(double value)
{
    if (value == _value)
        return;
    _value = value;
    emit valueChanged(_id, value);
    update();
}

ExprControl::ExprControl(int id, Editable *editable, bool showColorLink)
    : _id(id)
    , _updating(false)
    , _editable(editable)
{
    hbox = new QHBoxLayout(this);

    _colorLinkCB = new QCheckBox(this);
    _colorLinkCB->setFocusPolicy(Qt::NoFocus);
    connect(_colorLinkCB, SIGNAL(stateChanged(int)), this, SLOT(linkStateChange(int)));
    hbox->addWidget(_colorLinkCB);

    // see parser's specRegisterEditable
    // This is the variable name
    QString editableLabel = QString::fromStdString(_editable->name);
    _label = new QLabel();
    QFontMetrics _labelSize(_label->font());
    // Fix label appearance and word wrap, just in case -- amyspark
    // 45px gives us some breathing space
    _label->setMinimumWidth(60);
    _label->setText(tr("<b>%1</b>").arg(_labelSize.elidedText(editableLabel, Qt::TextElideMode::ElideRight, qMax(0, _label->width() - 15))));
    _label->setAutoFillBackground(true);
    hbox->addWidget(_label, 1);

    if (!showColorLink) {
        _colorLinkCB->setDisabled(true);
    } else {
        _colorLinkCB->setDisabled(false);
    }
}

void ExprControl::resizeEvent(QResizeEvent *)
{
    QString editableLabel = QString::fromStdString(_editable->name);
    QFontMetrics _labelSize(_label->font());
    _label->setText(tr("<b>%1</b>").arg(_labelSize.elidedText(editableLabel, Qt::TextElideMode::ElideRight, qMax(0, _label->width() - 15))));
}

void ExprControl::linkStateChange(int state)
{
    if (_updating)
        return;

    if (state == Qt::Checked) {
        emit linkColorLink(_id);
        emit linkColorEdited(_id, getColor());
    } else {
        emit linkColorLink(-1);
    }
}

void ExprControl::linkDisconnect(int newId)
{
    if (newId != _id) {
        _updating = true;
        _colorLinkCB->setChecked(false);
        _updating = false;
    }
}

NumberControl::NumberControl(int id, NumberEditable *editable)
    : ExprControl(id, editable, false)
    , _numberEditable(editable)
{
    auto *slider = new QHBoxLayout();
    // slider
    auto smin = editable->min;
    auto smax = editable->max;
    if (!_numberEditable->isInt) {
        smin *= 1e5;
        smax *= 1e5;
    }
    auto srange = smax - smin;
    _slider = new ExprSlider(Qt::Horizontal, this);
    _slider->setRange(int(smin), int(smax));
    _slider->setTickInterval(std::max(1, int(srange / 10)));
    _slider->setSingleStep(std::max(1, int(srange / 50)));
    _slider->setPageStep(std::max(1, int(srange / 10)));
    _slider->setFocusPolicy(Qt::ClickFocus);
    slider->addWidget(_slider, 3);
    // edit box
    _edit = new ExprLineEdit(0, this);
    slider->addWidget(_edit);
    hbox->addLayout(slider, 4);
    connect(_edit, SIGNAL(textChanged(int, const QString &)), SLOT(editChanged(int, const QString &)));
    connect(_slider, SIGNAL(valueChanged(int)), SLOT(sliderChanged(int)));
    // show current values
    updateControl();
}

void NumberControl::sliderChanged(int value)
{
    if (_updating)
        return;
    setValue(_numberEditable->isInt ? value : value * 1e-5);
}

void NumberControl::editChanged(int, const QString &text)
{
    if (_updating)
        return;
    bool ok = false;
    float val = text.toFloat(&ok);
    if (!ok)
        return;
    setValue(val);
}

void NumberControl::updateControl()
{
    _updating = true;
    int sliderval = int(_numberEditable->isInt ? _numberEditable->v : _numberEditable->v * 1e5);
    if (sliderval != _slider->value())
        _slider->setValue(sliderval);
    _edit->setText(QString(tr("%1")).arg(_numberEditable->v, 0, 'f', _numberEditable->isInt ? 0 : 3));
    _updating = false;
}

void NumberControl::setValue(double value)
{
    // dbgSeExpr<<"In setValue "<<_id<<value;
    if (fabs(_numberEditable->v - value) < 1e-5)
        return;
    _numberEditable->v = value;
    updateControl();
    emit controlChanged(_id);
}

VectorControl::VectorControl(int id, VectorEditable *editable)
    : ExprControl(id, editable, true)
    , _numberEditable(editable)
{
    auto *control = new QHBoxLayout();
    if (_numberEditable->isColor) {
        // CSwatchFrame has size 0 here! see below
        _swatch = new ExprCSwatchFrame(editable->v);
        connect(_swatch, SIGNAL(swatchChanged(QColor)), this, SLOT(swatchChanged(QColor)));
        control->addWidget(_swatch);
    }
    for (int i = 0; i < 3; i++) {
        auto *vbl = new QVBoxLayout();
        control->addLayout(vbl);
        vbl->setMargin(0);
        vbl->setSpacing(0);

        auto *edit = new ExprLineEdit(i, this);
        vbl->addWidget(edit);
        _edits[i] = edit;

        if (_numberEditable->isColor) {
            // piggy-back on the ExprLineEdit height to set the CSwatchFrame - amyspark
            auto width(edit->minimumSizeHint().width());
            auto height(edit->minimumSizeHint().height() + 6);
            _swatch->setMinimumWidth(width);
            _swatch->setMinimumHeight(height);
            _swatch->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        }

        auto *slider = new ExprChannelSlider(i, this);
        vbl->addWidget(slider);
        _sliders[i] = slider;
        // keep these, as channelSlider doesn't have a default height - amyspark
        slider->setFixedHeight(6);
        // set color
        static const std::array<QColor, 3> rgb = {QColor(128, 64, 64), QColor(64, 128, 64), QColor(64, 64, 128)};
        if (_numberEditable->isColor)
            slider->setDisplayColor(rgb[i]);

        connect(edit, SIGNAL(textChanged(int, const QString &)), SLOT(editChanged(int, const QString &)));
        connect(slider, SIGNAL(valueChanged(int, double)), SLOT(sliderChanged(int, double)));
    }
    hbox->addLayout(control, 4);
    // update controls
    updateControl();
}

void VectorControl::swatchChanged(QColor)
{
    KSeExpr::Vec3d color = _swatch->getValue();
    setValue(0, color[0]);
    setValue(1, color[1]);
    setValue(2, color[2]);
}

QColor VectorControl::getColor()
{
    return QColor::fromRgbF(clamp(_numberEditable->v[0], 0, 1), clamp(_numberEditable->v[1], 0, 1), clamp(_numberEditable->v[2], 0, 1));
}

void VectorControl::setColor(QColor color)
{
    setValue(0, color.redF());
    setValue(1, color.greenF());
    setValue(2, color.blueF());
}

void VectorControl::sliderChanged(int id, double value)
{
    if (_updating)
        return;
    setValue(id, _numberEditable->min + value * (_numberEditable->max - _numberEditable->min));
    if (_numberEditable->isColor)
        emit linkColorEdited(_id, getColor());
}

void VectorControl::editChanged(int id, const QString &text)
{
    if (_updating)
        return;
    bool ok = false;
    float val = text.toFloat(&ok);
    if (!ok)
        return;
    setValue(id, val);
}

void VectorControl::updateControl()
{
    //    //dbgSeExpr<<"In update control "<<_id;
    _updating = true;
    for (auto i = 0; i < 3; i++) {
        _edits[i]->setText(QString(tr("%1")).arg(_numberEditable->v[i], 0, 'f', 3));
    }
    auto min = _numberEditable->min;
    auto max = _numberEditable->max;
    for (auto i = 0; i < 3; i++) {
        _sliders[i]->setValue((_numberEditable->v[i] - min) / (max - min));
    }
    if (_numberEditable->isColor) {
        // dbgSeExpr<<"trying to set color";
        KSeExpr::Vec3d val = _numberEditable->v;
        auto r = clamp(val[0], 0, 1);
        auto g = clamp(val[1], 0, 1);
        auto b = clamp(val[2], 0, 1);
        auto lum = r * .2 + g * .7 + b * .1;
        // dbgSeExpr<<" rgb "<<r<<" "<<g<<" "<<b;
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(int(r * 255), int(g * 255), int(b * 255)));
        pal.setColor(QPalette::WindowText, (lum < 0.5) ? QColor(255, 255, 255) : QColor(0, 0, 0));
        _label->setPalette(pal);
    }
    _updating = false;
}

void VectorControl::setValue(int n, double value)
{
    if (n < 0 || n >= 3)
        return;
    if (fabs(_numberEditable->v[n] - value) < 1e-5)
        return;
    _numberEditable->v[n] = value;
    if (_swatch)
        _swatch->setValue(_numberEditable->v);
    updateControl();
    emit controlChanged(_id);
}

StringControl::StringControl(int id, StringEditable *editable)
    : ExprControl(id, editable, false)
    , _stringEditable(editable)
{
    // make line edit
    _edit = new QLineEdit();
    _edit->setFixedHeight(20);
    connect(_edit, SIGNAL(textChanged(const QString &)), SLOT(textChanged(const QString &)));
    // make a button if we are a file or directory
    if (_stringEditable->type == "file" || _stringEditable->type == "directory") {
        auto *button = new QPushButton();
        button->setFixedSize(20, 20);

        hbox->addWidget(_edit, 3);
        hbox->addWidget(button, 1);
        if (_stringEditable->type == "directory") {
            connect(button, SIGNAL(clicked()), SLOT(directoryBrowse()));
            button->setIcon(QIcon(QPixmap(directoryXPM.data())));
        } else if (_stringEditable->type == "file") {
            connect(button, SIGNAL(clicked()), SLOT(fileBrowse()));
            button->setIcon(QIcon(QPixmap(fileXPM.data())));
        }

    } else {
        hbox->addWidget(_edit, 3);
    }
    // update controls
    updateControl();
}

void StringControl::fileBrowse()
{
    ExprFileDialog dialog(this);
    dialog.setPreview();
    QString newFilename = dialog.getOpenFileName(tr("Please choose a file"), _edit->text(), tr("Images (*.tif *.tx *.jpg *.ptx *.png)"));
    if (!newFilename.isEmpty())
        _edit->setText(newFilename);
}

void StringControl::directoryBrowse()
{
    ExprFileDialog dialog(this);
    dialog.setPreview();
    QString newFilename = dialog.getExistingDirectory(tr("Please choose a file"), _edit->text());
    if (!newFilename.isEmpty())
        _edit->setText(newFilename);
}

void StringControl::updateControl()
{
    QString newText = QString::fromStdString(_stringEditable->v);
    _edit->setText(newText);
}

void StringControl::textChanged(const QString &newText)
{
    if (_updating)
        return;
    _stringEditable->v = newText.toStdString();
    emit controlChanged(_id);
}

CurveControl::CurveControl(int id, CurveEditable *editable)
    : ExprControl(id, editable, false)
    , _curveEditable(editable)
{
    _curve = new ExprCurve(this, tr("Pos:"), tr("Val:"), tr("Interp:"));

    const int numVal = _curveEditable->cvs.size();
    for (int i = 0; i < numVal; i++) {
        const KSeExpr::Curve<double>::CV &cv = _curveEditable->cvs[i];
        _curve->addPoint(cv._pos, cv._val, cv._interp);
    }
    hbox->addWidget(_curve, 4);
    connect(_curve->_scene, SIGNAL(curveChanged()), SLOT(curveChanged()));
    // unneded? updateControl();
}

void CurveControl::curveChanged()
{
    if (_curve && _curveEditable) {
        _curveEditable->cvs = _curve->_scene->_cvs;
        emit controlChanged(_id);
    }
}

CCurveControl::CCurveControl(int id, ColorCurveEditable *editable)
    : ExprControl(id, editable, true)
    , _curveEditable(editable)
{
    _curve = new ExprColorCurve(this, tr("Pos:"), tr("Val:"), tr("Interp:"));

    const int numVal = _curveEditable->cvs.size();
    for (int i = 0; i < numVal; i++) {
        const KSeExpr::Curve<KSeExpr::Vec3d>::CV &cv = _curveEditable->cvs[i];
        _curve->addPoint(cv._pos, cv._val, cv._interp);
    }
    hbox->addWidget(_curve, 4);
    connect(_curve->_scene, SIGNAL(curveChanged()), SLOT(curveChanged()));
    // unneeded? updateControl();
}

void CCurveControl::curveChanged()
{
    if (_curve && _curveEditable) {
        _curveEditable->cvs = _curve->_scene->_cvs;
        emit controlChanged(_id);
    }
}

QColor CCurveControl::getColor()
{
    return _curve->getSwatchColor();
}

void CCurveControl::setColor(QColor color)
{
    _curve->setSwatchColor(color);
}

class ExprGraphPreview : public QWidget
{
    Q_OBJECT
public:
    std::vector<float> x, y;
    std::vector<float> cpx, cpy;
    qreal xmin {}, xmax {}, ymin {}, ymax {}, dx {}, dy {};

    qreal win_xmin {}, win_xmax {}, win_ymin {}, win_ymax {}, win_dx {}, win_dy {};

    ExprGraphPreview(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        win_xmin = -1.;
        win_xmax = 2.;
        win_ymin = -1;
        win_ymax = 2.;
    }

    QPointF toScreen(qreal x, qreal y)
    {
        return {(x - win_xmin) * win_dx, height() - (y - win_ymin) * win_dy};
    }

    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QColor(255, 255, 255));
        win_xmin = xmin;
        win_xmax = xmax;
        win_ymin = ymin;
        win_ymax = ymax;
        auto percentXpad = .1 * (win_xmax - win_xmin);
        auto percentYpad = .1 * (win_ymax - win_ymin);
        win_xmin -= percentXpad;
        win_xmax += percentXpad;
        win_ymin -= percentYpad;
        win_ymax += percentYpad;

        // make space for text
        int x_pad_in_pixels = 25;
        int y_pad_in_pixels = 15;
        auto xpad = x_pad_in_pixels * (win_xmax - win_xmin) / (width() - x_pad_in_pixels);
        auto ypad = y_pad_in_pixels * (win_ymax - win_ymin) / (height() - y_pad_in_pixels);
        win_ymin -= ypad;
        win_xmax += xpad;

        win_dx = width() / (win_xmax - win_xmin);
        win_dy = height() / (win_ymax - win_ymin);

        // int h=height();
        QPainterPath path;
        QRectF fullarea(toScreen(win_xmin, win_ymax), toScreen(win_xmax, win_ymin));
        QBrush darkbrush(QColor(100, 100, 100), Qt::SolidPattern);
        QRectF area(toScreen(xmin, ymax), toScreen(xmax, ymin));
        QBrush brush(QColor(150, 150, 150), Qt::SolidPattern);
        // painter.fillRect(fullarea,darkbrush);
        // painter.setBrush(darkbrush);
        // painter.drawRoundedRect(fullarea,3,3);
        // painter.setBrush(QBrush());
        painter.fillRect(area, brush);
        if (!x.empty()) {
            path.moveTo(toScreen(x[0], y[0]));
            for (int i = 1; i < (int)x.size(); i++)
                path.lineTo(toScreen(x[i], y[i]));
        }
        QRectF right(toScreen(xmax, ymax), toScreen(win_xmax, ymin));
        QRectF bottom(toScreen(xmin, ymin), toScreen(xmax, win_ymin));

        painter.setPen(QColor(75, 50, 50));
        painter.drawPath(path);

        painter.setPen(QPen());
        painter.drawText(right, Qt::AlignTop | Qt::AlignLeft, QString(tr("%1")).arg(ymax, 0, 'f', 1));
        painter.drawText(right, Qt::AlignBottom | Qt::AlignLeft, QString(tr("%1")).arg(ymin, 0, 'f', 1));
        painter.drawText(bottom, Qt::AlignTop | Qt::AlignLeft, QString(tr("%1")).arg(xmin, 0, 'f', 1));
        painter.drawText(bottom, Qt::AlignTop | Qt::AlignRight, QString(tr("%1")).arg(xmax, 0, 'f', 1));

        painter.setBrush(QBrush(QColor(0, 0, 0), Qt::SolidPattern));
        for (size_t i = 0; i < cpx.size(); i++) {
            painter.drawEllipse(toScreen(cpx[i], cpy[i]), 2, 2);
        }
    }
};

// Editing widget for color swatch
ColorSwatchControl::ColorSwatchControl(int id, ColorSwatchEditable *editable)
    : ExprControl(id, editable, false)
    , _swatchEditable(editable)
    , _indexLabel(false)
{
    // include index labels if user specifies 'indices' as labelType
    if (_swatchEditable->labelType == "indices")
        _indexLabel = true;
    buildSwatchWidget();
}

void ColorSwatchControl::colorChanged(int index, KSeExpr::Vec3d value)
{
    if (_updating)
        return;
    if (index >= 0 && index < int(_swatchEditable->colors.size()))
        _swatchEditable->change(index, value);
    emit controlChanged(_id);
}

void ColorSwatchControl::colorAdded(int index, KSeExpr::Vec3d value)
{
    if (_updating)
        return;
    if (index >= 0 && index <= int(_swatchEditable->colors.size()))
        _swatchEditable->add(value); // add to end; TODO insert
    emit controlChanged(_id);
}

void ColorSwatchControl::colorRemoved(int index)
{
    if (_updating)
        return;
    if (index >= 0 && index < int(_swatchEditable->colors.size())) {
        _swatchEditable->remove(index);
        _swatch->deleteLater();
        _swatch = nullptr;
        buildSwatchWidget();
    }
    emit controlChanged(_id);
}

void ColorSwatchControl::buildSwatchWidget()
{
    _swatch = new ExprColorSwatchWidget(_indexLabel, this);
    connect(_swatch, SIGNAL(swatchChanged(int, KSeExpr::Vec3d)), this, SLOT(colorChanged(int, KSeExpr::Vec3d)));
    connect(_swatch, SIGNAL(swatchAdded(int, KSeExpr::Vec3d)), this, SLOT(colorAdded(int, KSeExpr::Vec3d)));
    connect(_swatch, SIGNAL(swatchRemoved(int)), this, SLOT(colorRemoved(int)));

    _updating = true;
    for (size_t i = 0; i < _swatchEditable->colors.size(); i++) {
        KSeExpr::Vec3d val = _swatchEditable->colors[i];
        _swatch->addSwatch(val, i);
    }
    _updating = false;
    hbox->addWidget(_swatch, 4);
}
