
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <sys/stat.h>
#include <sqlite3.h>
#include <cstring>
#include <string.h>
#include <regex>
#include <QDateEdit>
#include <string>

#include <QtWidgets>
#include <QMainWindow>
#include <QtCharts>
#include <QLineSeries>

using namespace std;

char currentFullName[100];
string currentFirstName, currentLastName;
string currentAthleteId;

sqlite3* db;

//QMainWindow* window;
//QWidget* window2;





MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);

    sqlite3_open("AthleteStatistics.db", &db); // open the database file "AthleteStatistics.db"

}

MainWindow::~MainWindow()
{
    delete ui;
}



static int callback(void *data, int argc, char **argv, char **azColName) {
    vector<string> *results = static_cast<vector<string>*>(data);
    for(int i = 0; i < argc; i++) {
        results->push_back(argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

void MainWindow::refreshAthletes(QComboBox *inputBoxHolder){
    inputBoxHolder->clear();

    string sql = "SELECT FirstName, LastName FROM Athletes ORDER BY FirstName";
    vector<string> results;
    char *errMsg = nullptr;
    sqlite3_exec(db, sql.c_str(), callback, &results, &errMsg);


    // results now contains the results of the query
    for (auto it = results.begin(); it != results.end(); std::advance(it, 2)) {
        const auto& result1 = *it;
        const auto& result2 = *(it + 1);
        const auto itemText = QString("%1 %2").arg(result1.c_str()).arg(result2.c_str());
        inputBoxHolder->addItem(itemText);
    }



}




void MainWindow::refreshEvents(QComboBox *inputBoxHolder) {

    inputBoxHolder->clear();

    string sql = "SELECT name FROM pragma_table_info('EventsAndTimes');";
    vector<string> results;
    char *errMsg = nullptr;
    sqlite3_exec(db, sql.c_str(), callback, &results, &errMsg);

    // Extract the distance from the event name using a regular expression
    regex distanceRegex("(\\d+)[mM]");

    vector<string> runningEvents;
    auto it = next(results.begin(), 4);
    for (; it != results.end(); ++it) {
        const auto& event = *it;
        smatch match;
        if (regex_search(event, match, distanceRegex)) {
            // This is a running event - add it to the runningEvents vector
            runningEvents.push_back(event);
        }
    }

    // Sort the running events by distance
    sort(runningEvents.begin(), runningEvents.end(),
         [&distanceRegex](const auto& lhs, const auto& rhs) {
             smatch lhsMatch, rhsMatch;
             regex_search(lhs, lhsMatch, distanceRegex);
             regex_search(rhs, rhsMatch, distanceRegex);
             return stoi(lhsMatch[1]) < stoi(rhsMatch[1]);
         });


    // Add the running events to the inputBoxHolder
    for (const auto& event : runningEvents) {
        const auto itemText = QString("%1").arg(event.c_str());
        inputBoxHolder->addItem(itemText);
    }

}


void MainWindow::on_viewAthletesButton_clicked()
{
    //code for viewing athletes
    ui->stackedWidget->setCurrentIndex(1);


    refreshAthletes(findChild<QComboBox*>("athleteListBox"));
    refreshAthletes(findChild<QComboBox*>("athleteListBox2"));


    //code that populates the athleteListBox with athletes
    //coppy above for anytime you need to refresh the list

}

//home from athleteViewer
void MainWindow::on_homeFromAthleteButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    QComboBox *inputBoxHolder;
    inputBoxHolder = findChild<QComboBox*>("athleteListBox");
    inputBoxHolder->clear();
}


//edit athlete button pressed
void MainWindow::on_editAthleteButton_clicked()
{
    string sql = "SELECT FirstName, LastName FROM Athletes ORDER BY FirstName";
    vector<string> results;
    char *errMsg = nullptr;
    sqlite3_exec(db, sql.c_str(), callback, &results, &errMsg);

    if(!results.empty()){
        QComboBox *inputBoxHolder;
        inputBoxHolder = findChild<QComboBox*>("athleteListBox");
        QString name = inputBoxHolder->currentText();

        //sets currentFullName
        strcpy_s(currentFullName, name.toUtf8().constData());



        //char *ptr;
        currentFirstName = strtok(currentFullName, " ");


        currentLastName = strtok(NULL, " ");


        QTextBrowser *athleteName;
        athleteName = findChild<QTextBrowser*>("athleteNameHolder");

        QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">p, li { white-space: pre-wrap; } hr { height: 1px; border-width: 0; } li.unchecked::marker { content: \"\\2610\"; } li.checked::marker { content: \"\\2612\"; }</style></head><body style=\" font-family:'Segoe UI'; font-size:1pt; font-weight:400; font-style:normal;\"><p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt;\">" + name + "</span></p></body></html>";

        athleteName->setHtml(html);


        string sql = "SELECT AthleteId FROM Athletes WHERE FirstName = '"+ currentFirstName +"' AND LastName = '"+ currentLastName +"'";
        vector<string> results;
        char *errMsg = nullptr;
        sqlite3_exec(db, sql.c_str(), callback, &results, &errMsg);


        for (string s : results) {
            currentAthleteId = s;
        }

        ui->stackedWidget->setCurrentIndex(2);
    }



}

//delete the current athlete and all events related to this athlete
void MainWindow::on_deleteAthleteButton_clicked()
{
    


    string sql = "PRAGMA FOREIGN_KEYS = ON;";
    char *errMsg = nullptr;
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    sql = "DELETE FROM Athletes WHERE AthleteId = '"+ currentAthleteId +"' ";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);




    refreshAthletes(findChild<QComboBox*>("athleteListBox"));
    refreshAthletes(findChild<QComboBox*>("athleteListBox2"));

    ui->stackedWidget->setCurrentIndex(1);


}



//button for the completion of adding times
void MainWindow::on_addTimeButton_clicked()
{
    QComboBox *inputBoxHolder;
    inputBoxHolder = findChild<QComboBox*>("eventSelectDropDown");
    string eventBeingAddedTo = inputBoxHolder->currentText().toStdString();


    QTextEdit *inputTextHolder;
    inputTextHolder = findChild<QTextEdit*>("timeInput");
    string timeInput = inputTextHolder->toPlainText().toStdString();
    inputTextHolder->clear();

    QDateEdit *inputDateHolder = new QDateEdit(this);
    inputDateHolder = findChild<QDateEdit*>("dateInput");
    string date = inputDateHolder->date().toString().toStdString();

    QTimeEdit *inputTimeHolder = new QTimeEdit(this);
    inputTimeHolder = findChild<QTimeEdit*>("timeOccuredInput");
    string timeOccured = inputTimeHolder->time().toString().toStdString();



    string sql = "INSERT INTO EventsAndTimes (AthleteId, DateOccurred, '"+ eventBeingAddedTo +"', TimeOccurred) VALUES ('"+ currentAthleteId +"', '"+ date +"', '"+ timeInput +"', '"+ timeOccured +"');";
    char *errMsg = nullptr;
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);




    ui->stackedWidget->setCurrentIndex(1);
}

//button to direct user to add times
void MainWindow::on_addTimesButton_clicked()
{

    //change date and time to current date and times
    QDateEdit *inputDateHolder = findChild<QDateEdit*>("dateInput");
    inputDateHolder->setDate(QDate::currentDate());

    QTimeEdit *inputTimeHolder = findChild<QTimeEdit*>("timeOccuredInput");
    inputTimeHolder->setTime(QTime::currentTime());


    refreshEvents(findChild<QComboBox*>("eventSelectDropDown"));
    refreshEvents(findChild<QComboBox*>("eventListBox2"));

    ui->stackedWidget->setCurrentIndex(3);
}

//home from createAthlete
void MainWindow::on_homeFromAthleteButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_backFromAddTimesButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}


void MainWindow::on_homeFromEventInfo_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_homeFromEditAthlete_clicked()
{
    refreshAthletes(findChild<QComboBox*>("athleteListBox"));
    refreshAthletes(findChild<QComboBox*>("athleteListBox2"));
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_backToMainFromVisualize_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_visualizeDataButton_clicked()
{
    refreshAthletes(findChild<QComboBox*>("athleteListBox"));
    refreshAthletes(findChild<QComboBox*>("athleteListBox2"));
    refreshEvents(findChild<QComboBox*>("eventSelectDropDown"));
    refreshEvents(findChild<QComboBox*>("eventListBox2"));

    ui->stackedWidget->setCurrentIndex(4);
}


void MainWindow::on_graphButton_clicked()
{
    string event;
    string athleteFullName;

    QComboBox *athleteListBox = findChild<QComboBox*>("athleteListBox2");
    athleteFullName = athleteListBox->currentText().toStdString();

    //split name into first and last
    char* athleteFullNameChar = const_cast<char*>(athleteFullName.c_str());
    string athleteFirstName = strtok(athleteFullNameChar, " ");
    string athleteLastName = strtok(NULL, " ");

    //select athleteID from searching First and Last Name of athletes table
    string sql = "SELECT AthleteId from Athletes WHERE FirstName = '" + athleteFirstName + "' AND LastName = '" + athleteLastName + "';";
    vector<string> results;
    char *errMsg = nullptr;
    sqlite3_exec(db, sql.c_str(), callback, &results, &errMsg);

    string athleteID;
    for(string s: results){
        athleteID = s;
    }

    QComboBox *eventListBox = findChild<QComboBox*>("eventListBox2");
    event = eventListBox->currentText().toStdString();

    QLineSeries *series = new QLineSeries();


    //clear results vector
    results.clear();

    //test sql for grabbing dates for x axis
    sql = "SELECT EventId, DateOccurred, TimeOccurred, \""+ event +"\" FROM 'EventsAndTimes' WHERE \""+ event +"\" IS NOT NULL AND AthleteId = " + athleteID + " ORDER BY CASE WHEN DateOccurred LIKE '%Jan%' THEN substr(DateOccurred, -4) || '-01-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Feb%' THEN substr(DateOccurred, -4) || '-02-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Mar%' THEN substr(DateOccurred, -4) || '-03-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Apr%' THEN substr(DateOccurred, -4) || '-04-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%May%' THEN substr(DateOccurred, -4) || '-05-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Jun%' THEN substr(DateOccurred, -4) || '-06-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Jul%' THEN substr(DateOccurred, -4) || '-07-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Aug%' THEN substr(DateOccurred, -4) || '-08-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Sep%' THEN substr(DateOccurred, -4) || '-09-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Oct%' THEN substr(DateOccurred, -4) || '-10-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Nov%' THEN substr(DateOccurred, -4) || '-11-' || substr(DateOccurred, -7, 2) WHEN DateOccurred LIKE '%Dec%' THEN substr(DateOccurred, -4) || '-12-' || substr(DateOccurred, -7, 2) ELSE DateOccurred END, TimeOccurred;";
    sqlite3_exec(db, sql.c_str(), callback, &results, &errMsg);

    //if ther are not enough results, notiy user and do not graph
    if(results.size() <= 4){

        QMessageBox messageBox;
        messageBox.critical(0, "ERROR", "Not enough data entries to graph");
        messageBox.setFixedSize(500, 200);

    }
    else{

        //0 is id, 1 is date, 2 is timeOccured, 3 is eventTime. increment 4i for each date

        for(int i = 3, j = 1, k = 0; i < results.size(); i += 4, j += 4, k+=4){
            double time = stod(results.at(i));
            QString qstr = QString::fromStdString(results.at(j));
            QDateTime dateTime = QDateTime::fromString(qstr, "ddd MMM d yyyy");


            double dateTimeInt = dateTime.toMSecsSinceEpoch() + stod(results.at(k));
            series->append(dateTimeInt, time);
        }

        QChart *chart = new QChart();
        chart->legend()->hide();
        chart->addSeries(series);

        QDateTimeAxis *axisX = new QDateTimeAxis();
        axisX->setTickCount(10);  // Adjust the number of ticks as needed
        axisX->setFormat("MM/dd/yyyy");  // Adjust the format as needed
        axisX->setTitleText("Date");
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Time (seconds)");  // Adjust the title as needed
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        chart->setTitle("Graph of Times");

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        QMainWindow *window = new QMainWindow();
        window->setWindowTitle("Line Graph");
        window->setCentralWidget(chartView);
        window->resize(800, 600);
        window->show();

    }
}


void MainWindow::on_actionNew_Event_triggered()
{
    //test to open a new window

    QWidget *window2 = new QWidget();

    //initialize window elements
    QTextBrowser *titleBar = new QTextBrowser();
        QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">p, li { white-space: pre-wrap; } hr { height: 1px; border-width: 0; } li.unchecked::marker { content: \"\\2610\"; } li.checked::marker { content: \"\\2612\"; }</style></head><body style=\" font-family:'Segoe UI'; font-size:1pt; font-weight:400; font-style:normal;\"><p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt;\">Enter Event Information</span></p></body></html>";
        titleBar->setHtml(html);
        titleBar->setFixedSize(440, 50);
    QTextBrowser *subTitleBar = new QTextBrowser();
        html = "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\"> p, li { white-space: pre-wrap; } hr { height: 1px; border-width: 0; } li.unchecked::marker { content: \"\2610\"; } li.checked::marker { content: \"\2612\"; } </style></head><body style=\" font-family:'Segoe UI'; font-size:1pt; font-weight:400; font-style:normal;\"> <p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt;\">Running Event</span></p></body></html>";
        subTitleBar->setHtml(html);
        subTitleBar->setFixedSize(440, 30);
    QTextEdit *distanceInMetersInput = new QTextEdit();
        distanceInMetersInput->setFixedSize(200, 30);
        distanceInMetersInput->setPlaceholderText("Enter Distance In Meters");
    QPushButton *createEventButton = new QPushButton();
        createEventButton->setText("Create");
        createEventButton->setFixedSize(100, 30);


    //horizontal nested layout
    QHBoxLayout *layoutH = new QHBoxLayout();
        layoutH->addWidget(distanceInMetersInput);
        layoutH->addWidget(createEventButton);
    //main layout
    QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(titleBar);
        layout->addWidget(subTitleBar);
        layout->addLayout(layoutH);
    window2->setLayout(layout);
    window2->setWindowTitle(QApplication::translate("windowlayout", "New Event"));
    window2->show();

    //functionality for buttons
    connect(createEventButton, &QPushButton::clicked, [this, distanceInMetersInput]() {
        string event = distanceInMetersInput->toPlainText().toStdString();
        event += "m";

        //refreshes dropdowns whom use the events
        refreshEvents(findChild<QComboBox*>("eventSelectDropDown"));
        refreshEvents(findChild<QComboBox*>("eventListBox2"));

        string sql = "ALTER TABLE EventsAndTimes ADD '" + event + "' DECIMAL(10,2);";
        char *errMsg = nullptr;
        sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

        distanceInMetersInput->clear();

        QMessageBox messageBox;
        messageBox.information(0, "SUCCESS", "Event Successfully Created");
        messageBox.setFixedSize(500, 200);
    });
}


void MainWindow::on_actionNew_Athlete_triggered()
{
    //test to open a new window
    QWidget *window2 = new QWidget();

    //initialize window elements
    QTextBrowser *titleBar = new QTextBrowser();
        QString html = "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">p, li { white-space: pre-wrap; } hr { height: 1px; border-width: 0; } li.unchecked::marker { content: \"\\2610\"; } li.checked::marker { content: \"\\2612\"; }</style></head><body style=\" font-family:'Segoe UI'; font-size:1pt; font-weight:400; font-style:normal;\"><p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt;\">Enter Athlete Information</span></p></body></html>";
        titleBar->setHtml(html);
        titleBar->setFixedSize(440, 50);
    QTextEdit *firstNameInput = new QTextEdit();
        firstNameInput->setFixedSize(200, 30);
        firstNameInput->setPlaceholderText("First");
    QTextEdit *lastNameInput = new QTextEdit();
        lastNameInput->setFixedSize(200, 30);
        lastNameInput->setPlaceholderText("Last");
    QComboBox *eventGroupInput = new QComboBox();
        eventGroupInput->addItem("Sprints");
        eventGroupInput->addItem("Distance");
        eventGroupInput->addItem("Jumps");
        eventGroupInput->addItem("Hurdles");
        eventGroupInput->addItem("Multi");
    QPushButton *createAthleteButton = new QPushButton();
        createAthleteButton->setText("Create");
        createAthleteButton->setFixedSize(100, 30);



    //initialize layouts
    QHBoxLayout *layoutH = new QHBoxLayout();
        layoutH->addWidget(firstNameInput);
        layoutH->addWidget(lastNameInput);
        layoutH->addWidget(eventGroupInput);

    QHBoxLayout *layoutH2 = new QHBoxLayout();
        layoutH2->addWidget(createAthleteButton);

    QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(titleBar);
        layout->addLayout(layoutH);
        layout->addLayout(layoutH2);


    window2->setLayout(layout);
    window2->setWindowTitle("New Athlete");
    window2->show();

    //functionality for buttons
    connect(createAthleteButton, &QPushButton::clicked, [firstNameInput, lastNameInput, eventGroupInput]() {

        string firstName, lastName, eventGroup;

        firstName = firstNameInput->toPlainText().toStdString();
        firstNameInput->setPlainText("");

        lastName = lastNameInput->toPlainText().toStdString();
        lastNameInput->setPlainText("");

        eventGroup = eventGroupInput->currentText().toStdString();
        eventGroupInput->setCurrentText("Sprints");


        string sql = "INSERT INTO ATHLETES (FirstName, LastName, Event) VALUES ('" + firstName + "', '" + lastName + "', '"+ eventGroup +"');";
        char *errMsg = nullptr;
        sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

        QMessageBox messageBox;
        messageBox.information(0, "SUCCESS", "Athlete Successfully Created");
        messageBox.setFixedSize(500, 200);
    });
}
