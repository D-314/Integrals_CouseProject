#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
private slots:
    void on_pushButton_calculate_clicked();

    void on_pushButton_about_clicked();

    void on_pushButton_file_hint_clicked();

    void on_pushButton_file_open_clicked();

    void on_doubleSpinBox_pred_1_valueChanged(double arg1);

    void on_doubleSpinBox_pred_2_valueChanged(double arg1);

    void on_pushButton_crack_pressed();

private:
    Ui::MainWindow *ui;
    double f(double x);
    double Fp(double x);
};

#endif // MAINWINDOW_H
