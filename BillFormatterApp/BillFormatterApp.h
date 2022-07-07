#pragma once

#include <QtWidgets/QMainWindow>
#include <qstandarditemmodel.h>
#include "ButtonItemDelegate.h"
#include "ui_BillFormatterApp.h"

using record_t = QList<QStandardItem*>;

enum class billCycleSelect_t
{
    bc1, bc2
};

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
    //bool fetchRecordsBc1();
    //bool fetchRecordsBc2();

private:
    bool fetchRecords(billCycleSelect_t bc);

private:
    Ui::BillFormatterAppClass ui;
    QStandardItemModel* model = nullptr;
    QStringList horizontalHeader;
    QStringList verticalHeader;
    ButtonItemDelegate* tableItemDelegate;
};
