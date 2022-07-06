#include "BillFormatterApp.h"
#include "ButtonItemDelegate.h"

BillFormatterApp::BillFormatterApp(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //Here we are setting your columns

    horizontalHeader.append("");
    horizontalHeader.append("ID");
    horizontalHeader.append("Time");
    horizontalHeader.append("Status");
    horizontalHeader.append("Document");

    // Here we are creating our model

    model = new QStandardItemModel();
    tableItemDelegate = new ButtonItemDelegate();

    model->setHorizontalHeaderLabels(horizontalHeader);
    model->setVerticalHeaderLabels(verticalHeader);

    ui.tableView->setModel(model); // This is necessary to display the data on table view
    ui.tableView->verticalHeader()->setVisible(false);
    ui.tableView->verticalHeader()->setDefaultSectionSize(10);
    ui.tableView->setShowGrid(false);

    ui.tableView->setItemDelegate(tableItemDelegate);
    ui.tableView->horizontalHeader()->setStretchLastSection(true);

    //// Here you can set your data in table view

    //QStandardItem* col0 = new QStandardItem("");
    //QStandardItem* col1 = new QStandardItem("ID");
    //QStandardItem* col2 = new QStandardItem("Time");
    //QStandardItem* col3 = new QStandardItem("Status");
    //QStandardItem* col4 = new QStandardItem("Document");

    //model->appendRow(QList<QStandardItem*>() << col1 << col2);
}

BillFormatterApp::~BillFormatterApp()
{}

BillFormatterApp* BillFormatterApp::instance()
{
    static BillFormatterApp app;
    return &app;
}

void BillFormatterApp::addRecord(const record_t& record)
{
    model->appendRow(record);
}

void BillFormatterApp::clearRecords()
{
    model->clear();
}

void BillFormatterApp::fetchRecords()
{
}

