#pragma once

#include <QtWidgets/QMainWindow>
#include <qstandarditemmodel.h>
#include "ButtonItemDelegate.h"
#include "ui_BillFormatterApp.h"

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "CmdClient.h"

using record_t = QList<QStandardItem*>;

class BillFormatterApp : public QMainWindow
{
    Q_OBJECT

public:
    BillFormatterApp(QWidget *parent = nullptr);
    virtual ~BillFormatterApp() = default;

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
    bool fetchRecords(size_t offset, size_t count);
    size_t get_records_count();
    void create_paging();

private:
    Ui::BillFormatterAppClass ui;
    QStandardItemModel* model = nullptr;
    QStringList horizontalHeader;
    QStringList verticalHeader;
    ButtonItemDelegate* tableItemDelegate;

    zmq::context_t context;
    CmdClient req_sender;

    billCycleSelect_t active_bill_cycle = billCycleSelect::bc1;

    size_t record_count = 0;
    QList<QCommandLinkButton*> paging_buttons;
    size_t paging_index_begin = 0;
    size_t paging_page_count = 5;
    size_t paging_page_length = 10;
};
