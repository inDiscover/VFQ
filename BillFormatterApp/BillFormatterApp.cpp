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

    create_paging();
}

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

    ui.tableView->setColumnWidth(0, 20);
    ui.tableView->setColumnWidth(1, 50);
    ui.tableView->setColumnWidth(2, 180);
}

bool BillFormatterApp::fetchRecords(billCycleSelect_t bc)
{
    // TODO Introduce handling of two separate bill cycles
    switch (bc)
    {
        case billCycleSelect_t::bc1:
        case billCycleSelect_t::bc2:
            fetchRecords(0, paging_page_length);
    }
    return true;
}

bool BillFormatterApp::fetchRecords(size_t offset, size_t count)
{
    clearRecords();

    message_data_t data;
    ReqRecords rec_req(nullptr, offset, count);
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
        std::getline(sin, items[4]);
        std::for_each(items.begin(), items.end(), [&](const auto& el) { cells.push_back(new QStandardItem(el.data())); });

        record_t record = record_t() << cells[0] << cells[1] << cells[2] << cells[3] << cells[4];
        addRecord(record);
    }

    return true;
}

size_t BillFormatterApp::get_records_count()
{
    message_data_t data;
    ReqCount req{};
    req_sender.request_receive(req, data);

    if (!data.empty())
    {
        char* pend = nullptr;
        record_count = std::strtoul(data[0].data(), &pend, 10);
        if (data[0].data() == pend)
        {
            // Failed to convert string to number
            record_count = 0;
        }
    }

    return record_count;
}

void BillFormatterApp::create_paging()
{
    record_count = get_records_count();
    auto recCntText = QString::number(record_count) + QLatin1String(" records");
    ui.recCntButton->setText(recCntText);

    auto page_number = 0u;
    for (auto rec_number=0u; rec_number<record_count && page_number<paging_page_count; rec_number+=paging_page_length)
    {
        auto* page_button = new QCommandLinkButton(QString::number(++page_number));
        page_button->setIconSize({0, 0});
        page_button->setMinimumSize({10, 0});
        page_button->setMaximumSize({60, 50});
        connect(page_button, &QCommandLinkButton::clicked, this, [=] { fetchRecords(rec_number, this->paging_page_length); });
        paging_buttons.append(page_button);
        ui.pagingPanel->addWidget(page_button);
    }
    ui.pagingPanel->addStretch();
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

