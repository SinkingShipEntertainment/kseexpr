/*
* Copyright Disney Enterprises, Inc.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License
* and the following modification to it: Section 6 Trademarks.
* deleted and replaced with:
*
* 6. Trademarks. This License does not grant permission to use the
* trade names, trademarks, service marks, or product names of the
* Licensor and its affiliates, except as required for reproducing
* the content of the NOTICE file.
*
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
*
* @file ExprDialog.h
* @brief A basic editor/browser/previewer for expression editing
* @author  jlacewel
*/
#ifndef _MY_EXPR_EDITOR_H
#define _MY_EXPR_EDITOR_H

#include <QFileDialog>
#include <QGLWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QObject>
#include <QPalette>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSplitter>

#include <fstream>
#include <iostream>

#include "ExprEditor.h"

class ExprGrapherWidget;
class ExprBrowser;
class QTabWidget;

class ExprDialog : public QDialog {
    Q_OBJECT

  public:
    ExprEditor* editor;
    ExprBrowser* browser;

  private:
    ExprGrapherWidget* grapher;
    QLabel* previewCommentLabel;
    QPushButton* acceptButton;
    QPushButton* cancelButton;
    ExprControlCollection* controls;

    QPushButton *applyButton, *previewButton, *saveButton, *saveAsButton;
    QPushButton *saveLocalButton, *clearButton;
    QLineEdit* helpFindBox;
    QTimer* showEditorTimer;
    QTextBrowser* helpBrowser;
    QTextCursor cursor;
    QString prevFind;
    int _currentEditorIdx;
    QStringList history;
    int currhistitem;
    QString currentexprfile;

  public:
    ExprDialog(QWidget* parent);

    std::string getExpressionString() { return editor->getExpr(); }

    void setExpressionString(const std::string& str) { editor->setExpr(str); }

    void show();
    int exec();

    // Show the Nth editor dialog
    void showEditor(int idx);

  private:
    void setupHelp(QTabWidget* tab);
    void enableBackForwards();

  protected:
    void keyPressEvent(QKeyEvent* event);
    void findHelper(QTextDocument::FindFlags flags);
    void closeEvent(QCloseEvent* event);

  signals:
    void preview();
    void expressionApplied();
    void dialogClosed();
    void forwardAvailable(bool);
    void backwardAvailable(bool);
  private slots:
    void previewExpression();
    void verifiedApply();
    void verifiedAccept();
    void reloadExpression();
    void findNextInHelp();
    void findPrevInHelp();
    void _showEditor();
    void selectionChanged(const QString& str);
    void histBackward();
    void histForward();
    void histAdd();
  public slots:

    void applyExpression();

    void clearExpression();

    void reject();
};

#endif
