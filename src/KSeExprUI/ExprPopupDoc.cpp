// SPDX-FileCopyrightText: 2011-2019 Disney Enterprises, Inc.
// SPDX-License-Identifier: LicenseRef-Apache-2.0
#include "ExprPopupDoc.h"
#include <QLabel>
#include <QHBoxLayout>

ExprPopupDoc::ExprPopupDoc(QWidget* parent, const QPoint& placecr, const QString& msg) {
    Q_UNUSED(parent);
    label = new QLabel(msg);
    QHBoxLayout* layout = new QHBoxLayout;
    setLayout(layout);
    layout->addWidget(label);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint |
                   Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);
    move(placecr);
    raise();
    show();
}

void ExprPopupDoc::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    hide();
}