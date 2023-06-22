#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <sqlite3.h>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    //create database if not created
    sqlite3* db;
    sqlite3_open("AthleteStatistics.db", &db); // open the database file "AthleteStatistics.db"


    const char *sql = "PRAGMA FOREIGN_KEYS = ON;";
    char *errMsg = nullptr;
    sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);


    sql = "CREATE TABLE if NOT EXISTS Athletes (AthleteId INTEGER PRIMARY KEY, FirstName TEXT, LastName TEXT, Event TEXT);";
    sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

    sql = "CREATE TABLE if NOT EXISTS EventsAndTimes (EventId INTEGER PRIMARY KEY, AthleteId INTEGER, DateOccurred DATE, TimeOccurred TIME, FOREIGN KEY(AthleteId) REFERENCES Athletes(AthleteId) ON DELETE CASCADE);";
    sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

    sqlite3_close(db);
    return a.exec();
}
