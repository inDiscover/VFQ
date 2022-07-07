#include "BillFormatterApp.h"
#include "ButtonItemDelegate.h"

BillFormatterApp::BillFormatterApp(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //connect(ui.bc1Button, SIGNAL(clicked()), this, SLOT(fetchRecordsBc1()));
    //connect(ui.bc1Button, &QPushButton::clicked, this, &BillFormatterApp::fetchRecordsBc1);
    connect(ui.bc1Button, &QPushButton::clicked, this, [this] { fetchRecords(billCycleSelect_t::bc1); });
    connect(ui.bc2Button, &QPushButton::clicked, this, [this] { fetchRecords(billCycleSelect_t::bc2); });

	clearRecords();
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
	if (model)
	{
		model->clear();
	}

    model = new QStandardItemModel();
    tableItemDelegate = new ButtonItemDelegate();

    model->setHorizontalHeaderLabels(horizontalHeader);
    model->setVerticalHeaderLabels(verticalHeader);

    // Here we are creating our model

    ui.tableView->setModel(model); // This is necessary to display the data on table view
    ui.tableView->verticalHeader()->setVisible(false);
    ui.tableView->verticalHeader()->setDefaultSectionSize(10);
    ui.tableView->setShowGrid(false);

    ui.tableView->setItemDelegate(tableItemDelegate);
    ui.tableView->horizontalHeader()->setStretchLastSection(true);

    //Here we are setting your columns

    horizontalHeader.append("");
    horizontalHeader.append("ID");
    horizontalHeader.append("Time");
    horizontalHeader.append("Status");
    horizontalHeader.append("Document");
}

bool BillFormatterApp::fetchRecords(billCycleSelect_t bc)
{
	QStandardItem* col0 = nullptr;
	QStandardItem* col1 = nullptr;
	QStandardItem* col2 = nullptr;
	QStandardItem* col3 = nullptr;
	QStandardItem* col4 = nullptr;

	clearRecords();

	for (auto i{ 0 }; i < 4; ++i)
	{
		switch (bc)
		{
		case billCycleSelect_t::bc1:
			col0 = new QStandardItem("");
			col1 = new QStandardItem("0000");
			col2 = new QStandardItem("2022-01-02");
			col3 = new QStandardItem("Success");
			col4 = new QStandardItem("bill00.html");
			break;
		case billCycleSelect_t::bc2:
			col0 = new QStandardItem("");
			col1 = new QStandardItem("0000");
			col2 = new QStandardItem("2022-01-16");
			col3 = new QStandardItem("Failed");
			col4 = new QStandardItem("bill00.html");
			break;
		default:
			return false;
		}

		record_t record = record_t() << col0 << col1 << col2 << col3 << col4;
		addRecord(record);
	}

	return true;
}

//bool BillFormatterApp::fetchRecordsBc1()
//{
//    return fetchRecords(billCycleSelect_t::bc1);
//}
//
//bool BillFormatterApp::fetchRecordsBc2()
//{
//    return fetchRecords(billCycleSelect_t::bc2);
//}

