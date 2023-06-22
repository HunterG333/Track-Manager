#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_viewAthletesButton_clicked();

    void on_homeFromAthleteButton_clicked();

    void on_homeFromAthleteButton_2_clicked();

    void on_editAthleteButton_clicked();

    void on_deleteAthleteButton_clicked();

    void on_homeFromEditAthlete_clicked();

    void on_addTimesButton_clicked();

    void on_backFromAddTimesButton_clicked();

    void on_homeFromEventInfo_clicked();

    void on_addTimeButton_clicked();

    void refreshEvents(QComboBox *inputBoxHolder);

    void refreshAthletes(QComboBox *inputBoxHolder);

    void on_backToMainFromVisualize_clicked();

    void on_visualizeDataButton_clicked();

    void on_graphButton_clicked();

    void on_actionNew_Event_triggered();

    void on_actionNew_Athlete_triggered();

private:
    Ui::MainWindow *ui;
    bool fileExists(const std::string& filename);
};
#endif // MAINWINDOW_H
