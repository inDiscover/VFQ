#pragma once

#include <QtWidgets/QMainWindow>
#include <qstandarditemmodel.h>
#include "ButtonItemDelegate.h"
#include "ui_BillFormatterApp.h"

using record_t = QList<QStandardItem*>;

class BillFormatterApp : public QMainWindow
{
    Q_OBJECT

public:
    BillFormatterApp(QWidget *parent = nullptr);
    virtual ~BillFormatterApp();

public:
    static BillFormatterApp* instance();

public:
    void addRecord(const record_t& record);
    void clearRecords();

public slots:
    void fetchRecords();

private:
    Ui::BillFormatterAppClass ui;
    QStandardItemModel* model;
    QStringList horizontalHeader;
    QStringList verticalHeader;
    ButtonItemDelegate* tableItemDelegate;
};
