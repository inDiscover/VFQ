#include "BillFormatterApp.h"
#include "ButtonItemDelegate.h"
#include "Commands.h"
#include <array>

BillFormatterApp::BillFormatterApp(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //connect(ui.bc1Button, SIGNAL(clicked()), this, SLOT(fetchRecordsBc1()));
    //connect(ui.bc1Button, &QPushButton::clicked, this, &BillFormatterApp::fetchRecordsBc1);
    connect(ui.bc1Button, &QPushButton::clicked, this, [this] { fetchRecords(billCycleSelect_t::bc1); });
    connect(ui.bc2Button, &QPushButton::clicked, this, [this] { fetchRecords(billCycleSelect_t::bc2); });

    //Here we are setting your columns

    horizontalHeader.append("");
    horizontalHeader.append("ID");
    horizontalHeader.append("Time");
    horizontalHeader.append("Status");
    horizontalHeader.append("Document");

	req_sender = CmdClient(context);
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
}

bool BillFormatterApp::fetchRecords(billCycleSelect_t bc)
{
	clearRecords();

	//QStandardItem* col0 = nullptr;
	//QStandardItem* col1 = nullptr;
	//QStandardItem* col2 = nullptr;
	//QStandardItem* col3 = nullptr;
	//QStandardItem* col4 = nullptr;

	//for (auto i{ 0 }; i < 4; ++i)
	//{
	//	switch (bc)
	//	{
	//	case billCycleSelect_t::bc1:
	//		col0 = new QStandardItem("");
	//		col1 = new QStandardItem("0000");
	//		col2 = new QStandardItem("2022-01-02");
	//		col3 = new QStandardItem("Success");
	//		col4 = new QStandardItem("bill00.html");
	//		break;
	//	case billCycleSelect_t::bc2:
	//		col0 = new QStandardItem("");
	//		col1 = new QStandardItem("0000");
	//		col2 = new QStandardItem("2022-01-16");
	//		col3 = new QStandardItem("Failed");
	//		col4 = new QStandardItem("bill00.html");
	//		break;
	//	default:
	//		return false;
	//	}

	//	record_t record = record_t() << col0 << col1 << col2 << col3 << col4;
	//	addRecord(record);
	//}

	constexpr auto cnt = 10;
	message_data_t data;
	ReqRecords rec_req(nullptr, 0, cnt);
	req_sender.request_receive(rec_req, data);

	for (auto i{ 0 }; i < data.size(); ++i)
	{
		std::istringstream sin(data[i]);
		std::array<std::string, 5> items;
		std::vector<QStandardItem*> cells;
		cells.reserve(items.size());

		// First cell contains the buttons
		items[0] = std::string();
		// Read the records elements
		sin >> items[1] >> items[2] >> items[3];
		items[4].resize(1024);
		sin.getline(items[4].data(), items[4].size());
		auto it = cells.begin();
		std::for_each(items.begin(), items.end(), [&](const auto& el) { cells.push_back(new QStandardItem(el.data())); });

		record_t record = record_t() << cells[0] << cells[1] << cells[2] << cells[3] << cells[4];
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

