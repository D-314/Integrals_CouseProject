#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <QString>
#define KOEF_A ui->doubleSpinBox_koef_a->value()
#define KOEF_B ui->doubleSpinBox_koef_b->value()
#define KOEF_C ui->doubleSpinBox_koef_c->value()
#define PRED_1 ui->doubleSpinBox_pred_1->value()
#define PRED_2 ui->doubleSpinBox_pred_2->value()
#define INTERV ui->spinBox_interv->value()

double MainWindow::f(double x)  //функция
{
    double y = 0.0;
    switch (ui->comboBox_func->currentIndex()) // С помощью ф-и currentIndex проверяем id выбранной функции
    {
    case 0:
    {
        y=(KOEF_A*pow(x,3)+KOEF_B*x+KOEF_C);
        break;
    }
    case 1:
    {
        y=(KOEF_A*sin(x+KOEF_B)+KOEF_C);
        break;
    }
    case 2:
    {
        y=(KOEF_A*log(fabs(x+KOEF_B))+KOEF_C);
        break;
    }
    };
    return y;
}
double MainWindow::Fp(double x) //первообразная функции
{
    double y = 0.0;
    switch (ui->comboBox_func->currentIndex()) // С помощью ф-и currentIndex проверяем id выбранной функции
    {
    case 0:
    {
        y=(KOEF_A*pow(x,4)/4)+KOEF_B*pow(x,2)/2+KOEF_C*x;
        break;
    }
    case 1:
    {
        y=(KOEF_C*x-KOEF_A*cos(KOEF_B+x));
        break;
    }
    case 2:
    {
        y=(KOEF_A*((KOEF_B+x)*log(fabs(KOEF_B+x))-(KOEF_B+x))+KOEF_C*x);
        break;
    }
    };
    return y;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon)));
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iMultiSelect);
    ui->customPlot->axisRect()->setupFullAxesBox();
    connect( ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect( ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    ui->comboBox_method->setCurrentIndex(ui->comboBox_method->maxCount()-1);
    while (ui->comboBox_method->currentIndex() == -1) {
        ui->comboBox_method->setMaxCount(ui->comboBox_method->maxCount()-1);
        ui->comboBox_method->setCurrentIndex(ui->comboBox_method->maxCount()-1);
    }
    ui->comboBox_method->setCurrentIndex(0);
    ui->progressBar_calculate->hide();
    ui->pushButton_crack->hide();

    QFile file("config.txt"); //загрузка конфига, если он лежит в папке с программой
    if (file.open(QIODevice::ReadOnly))
    {
        QString conf = file.readAll();
        file.close();
        QStringList param = conf.split(10);
        param.size();
        bool err[4];
        for (int i=0;((i<param.size()) && (i<4)); i++) {
            param.at(i).toDouble(&err[i]);
            if (err[i]) {
                                switch (i) {
                case 0:  //метод интегрирования по умолчанию (при запуске)
                {
                    int par = param.at(i).toDouble();
                    ui->comboBox_method->setCurrentIndex(std::abs(par) % ui->comboBox_method->maxCount());
                    ui->label_method->hide();
                    ui->comboBox_method->hide();
                    break;
                }
                case 1: //видимость выбора метода интегрирования (если не задано - скрыт, 0 - отключено, 1 - виден)
                {
                    if (param.at(i).toDouble() > 0) { //видны редактируемые элементы
                        ui->label_method->show();
                        ui->comboBox_method->show();
                    }
                    else { //при нуле видны деактивированные элементы
                        ui->label_method->show();
                        ui->label_method->setEnabled(0);
                        ui->comboBox_method->show();
                        ui->comboBox_method->setEnabled(0);
                    }
                    break;
                }
                case 2: //возможность снятия ограничений
                {
                    if (param.at(i).toDouble() > 0) {
                        ui->pushButton_crack->show();
                    }
                    break;
                }
                case 3: //возможность снятия ограничений
                {
                    if (param.at(i).toDouble() > 0) {
                        ui->pushButton_about->setEnabled(0);
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }
        if (!(ui->comboBox_method->isVisible())) {this->resize(this->geometry().width(),this->geometry().height()-50);}
    }
    this->setWindowTitle(("Численное интегрирование методом "+ui->comboBox_method->currentText()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_calculate_clicked()
{
    ui->groupBox_input->setEnabled(0);
    ui->pushButton_calculate->hide();
    ui->progressBar_calculate->setValue(0);
    ui->progressBar_calculate->show();
    ui->customPlot->clearGraphs();//Если нужно, то очищаем все графики
    if(!(PRED_2>PRED_1) && !(PRED_2<PRED_1)) {
        ui->doubleSpinBox_pred_1->setValue(PRED_1-10);
        ui->doubleSpinBox_pred_2->setValue(PRED_2+10);
    }
    if(PRED_2<PRED_1)
    {
        ui->doubleSpinBox_pred_1->setValue(PRED_1+PRED_2);
        ui->doubleSpinBox_pred_2->setValue(PRED_1-PRED_2);
        ui->doubleSpinBox_pred_1->setValue(PRED_1-PRED_2);
    }

    int N=2000;
    double h = (std::abs(PRED_1-PRED_2))/(N); //вычисляем длину одного интервала
    double minY = f(PRED_1), maxY = f(PRED_1);
    ui->customPlot->addGraph();//основной график
    //задаем параметра граффика
    ui->customPlot->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 30))); //цвет заполнения
    ui->customPlot->graph(0)->setPen(QPen(Qt::red));//цвет линии
    ui->customPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsLine);//стиль рисования граффика

    for (double x=PRED_1; x<=PRED_2; x+=h) { //Пробегаем по всем точкам
        ui->customPlot->graph(0)->addData(x, f(x)); //добавляем точки на график
        if ((f(x) != INFINITY) || (-f(x) != INFINITY)) {
            if (f(x)<minY) {minY = f(x);}   //находим минимальное значение аункции
            if (f(x)>maxY) {maxY = f(x);}   //находим максимальное значение аункции
        }
    }
    //Установим область, которая будет показываться на графике
    ui->customPlot->xAxis->setRange(PRED_1, PRED_2);//Для оси Ox
    ui->customPlot->yAxis->setRange(minY, maxY);    //Для оси Oy

    h = (std::abs(PRED_1-PRED_2))/(INTERV);
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setBrush(QBrush(QColor(0, 0, 255, 30))); //цвет заполнения
    ui->customPlot->graph(1)->setPen(QPen(Qt::blue)); //цвет линии
    if (INTERV<41) { //показ точек, значения которых были взяты за высоту при рассчете интеграла заданным методом
        ui->customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    }
    double integral_p=0, integral_t=Fp(PRED_2)-Fp(PRED_1);
    QString met;
    //весь код, завсящий от выбранного метода рассчета находится здесь//

    switch (ui->comboBox_method->currentIndex()) { //в зависимости от выбранного метода производим рассчет
    case 0://метод левых прямоугольников
    {
        ui->customPlot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);//стиль рисования граффика
        for (double x=PRED_1; x<=PRED_2+h/2; x+=h) { //Пробегаем по всем точкам
            ui->customPlot->graph(1)->addData(x, f(x)); //добавляем точки на график
            ui->progressBar_calculate->setValue(50*(std::abs(x-PRED_1)/std::abs(PRED_2-PRED_1)));
        }

        for (int i=0; i<INTERV; i++) {
            if ((f(PRED_1+(i*h)) != INFINITY) && (-f(PRED_1+(i*h)) != INFINITY)) { //костыль, что бы значение интеграла не получалось бесконечным из заотрезка с бесконечно большой площадью
                integral_p += h*f((PRED_1+(i*h)));
                ui->progressBar_calculate->setValue(50+50*i/INTERV);
            }
        }

        met="правых прямоуг.";

        break;
    }
    case 1://метод центральных прямоугольников
    {
        ui->customPlot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
        for (double x=(PRED_1-h/2); x<=(PRED_2+h); x+=h) {
            ui->customPlot->graph(1)->addData(x, f(x));
            ui->progressBar_calculate->setValue(50*std::abs(x-PRED_1)/std::abs(PRED_2-PRED_1));
        }

        for (int i=0; i<INTERV; i++) {
            if ((f(PRED_1+(((1+2*i)*h)/2)) != INFINITY) && (-f(PRED_1+(((1+2*i)*h)/2)) != INFINITY)) {
                integral_p  += h*f(PRED_1+(((1+2*i)*h)/2));
                ui->progressBar_calculate->setValue(50+50*i/INTERV);
            }
        }

        met="центр. прямоуг.";

        break;
    }
    case 2://метод правых прямоугольников
    {
        ui->customPlot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepRight);
        for (double x=PRED_1; x<=(PRED_2+h/2); x+=h) {
            ui->customPlot->graph(1)->addData(x, f(x));
            ui->progressBar_calculate->setValue(50*std::abs(x-PRED_1)/std::abs(PRED_2-PRED_1));
        }

        for (int i=1; i<=INTERV; i++) {
            if ((f(PRED_1+(i*h)) != INFINITY) && (-f(PRED_1+(i*h)) != INFINITY)) {
                integral_p += h*f(PRED_1+(i*h));
                ui->progressBar_calculate->setValue(50+50*i/INTERV);
            }
        }

        met="левых прямоуг.";

        break;
    }
    case 3://метод трапеций
    {
        ui->customPlot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsLine);
        for (double x=PRED_1; x<=(PRED_2+h); x+=h) {
            ui->customPlot->graph(1)->addData(x, f(x));
            ui->progressBar_calculate->setValue(50*std::abs(x-PRED_1)/std::abs(PRED_2-PRED_1));
        }

        for (int i=0; i<INTERV; i++) {
            if ((f(PRED_1+(i*h)) != INFINITY) && (-f(PRED_1+(i*h)) != INFINITY) && (f(PRED_1+((1+i)*h)) != INFINITY) && (-f(PRED_1+((1+i)*h)) != INFINITY)) {
                integral_p += h*(f(PRED_1+(i*h))+f(PRED_1+((1+i)*h)))/2;
                ui->progressBar_calculate->setValue(50+50*i/INTERV);
            }
        }

        met="трапеций";

        break;
    }
    case 4://метод парабол (кратный 2), требуется 3 точки или 2 интервала
    {
        ui->spinBox_interv->setValue(INTERV+INTERV%2);
        h = (std::abs(PRED_1-PRED_2))/(INTERV);

        for (int i=0; i<INTERV; i+=2) {
            if ((f(PRED_1+(i*h)) != INFINITY) && (-f(PRED_1+(i*h)) != INFINITY) && (f(PRED_1+((1+i)*h)) != INFINITY) && (-f(PRED_1+((1+i)*h)) != INFINITY) && (f(PRED_1+((2+i)*h)) != INFINITY) && (-f(PRED_1+((2+i)*h)) != INFINITY)) {
                integral_p += h/3*(f(PRED_1+(i*h))+4*f(PRED_1+((1+i)*h))+f(PRED_1+((2+i)*h)));
                ui->progressBar_calculate->setValue(100*i/INTERV);
            }
        }

        met="парабол";

        break;
    }
    case 5://метод Симпсона 3/8 (кратный 3), требуется 4 точки или 3 интервала
    {
        if (INTERV>3) {
            if ((INTERV%3 > 1) && (INTERV < 500)) {
                ui->spinBox_interv->setValue(INTERV+INTERV%3);
            } else {
                ui->spinBox_interv->setValue(INTERV-INTERV%3);
            }
        } else {
            ui->spinBox_interv->setValue(3);
        }
        h = (std::abs(PRED_1-PRED_2))/(INTERV);

        for (int i=0; i<INTERV; i+=3) {
            if ((f(PRED_1+(i*h)) != INFINITY) && (-f(PRED_1+(i*h)) != INFINITY) && (f(PRED_1+((1+i)*h)) != INFINITY) && (-f(PRED_1+((1+i)*h)) != INFINITY) && (f(PRED_1+((2+i)*h)) != INFINITY) && (-f(PRED_1+((2+i)*h)) != INFINITY) && (f(PRED_1+((3+i)*h)) != INFINITY) && (-f(PRED_1+((3+i)*h)) != INFINITY)) {
                integral_p += ((3*h)/8)*(f(PRED_1+(i*h))+3*f(PRED_1+((1+i)*h))+3*f(PRED_1+((2+i)*h))+f(PRED_1+((3+i)*h)));
                ui->progressBar_calculate->setValue(100*i/INTERV);
            }
        }

        met="Симпсона 3/8";

        break;
    }
    case 6://метод Буля (кратный 4), тоебуется 5 точек или 4 интервала
    {
        ui->spinBox_interv->setValue(INTERV+(4-INTERV%4)%4);
        h = (std::abs(PRED_1-PRED_2))/(INTERV);

        for (int i=0; i<INTERV; i+=4) {
            if ((f(PRED_1+(i*h)) != INFINITY) && (-f(PRED_1+(i*h)) != INFINITY) && (f(PRED_1+((1+i)*h)) != INFINITY) && (-f(PRED_1+((1+i)*h)) != INFINITY) && (f(PRED_1+((2+i)*h)) != INFINITY) && (-f(PRED_1+((2+i)*h)) != INFINITY) && (f(PRED_1+((3+i)*h)) != INFINITY) && (-f(PRED_1+((3+i)*h)) != INFINITY) && (f(PRED_1+((4+i)*h)) != INFINITY) && (-f(PRED_1+((4+i)*h)) != INFINITY)) {
                integral_p += ((2*h)/45)*(7*f(PRED_1+(i*h))+32*f(PRED_1+((1+i)*h))+12*f(PRED_1+((2+i)*h))+32*f(PRED_1+((3+i)*h))+7*f(PRED_1+((4+i)*h)));
                ui->progressBar_calculate->setValue(100*i/INTERV);
            }
        }

        met="Буля";

        break;
    }
    default:
    {
        integral_p=INFINITY;
        QMessageBox::critical(this, "Ошибка", "Не выбран существующий метод");
        met="не выбран";
        break;
    }
    }
    this->setWindowTitle(("Численное интегрирование методом "+ui->comboBox_method->currentText()));

    //если интервалов меньше или равно 60, то показываем разделители
    if (INTERV<=60) {
        ui->customPlot->addGraph();
        ui->customPlot->graph(2)->setPen(QPen(Qt::blue));
        ui->customPlot->graph(2)->setLineStyle(QCPGraph::LineStyle::lsImpulse);
        for (double x=PRED_1; x<(PRED_2+h); x+=h) {ui->customPlot->graph(2)->addData(x, f(x));}
    }

    ui->customPlot->replot();//отрисовываем графики

    ui->result_textBrowser->setText(QString("Метод %1:\t%2 \nТочное значение интеграла:\t%3 \nАбсолютная погрешность:\t%4 \nОтносительная погрешность:\t%5%").arg(met).arg(integral_p).arg(integral_t).arg(std::abs(integral_t-integral_p)).arg((std::abs(integral_t-integral_p)/integral_t)*100));

    ui->pushButton_calculate->show();
    ui->progressBar_calculate->hide();
    ui->groupBox_input->setEnabled(1);

    ui->groupBox_input->setFocus();
}

void MainWindow::on_pushButton_file_open_clicked()
{
    #define maxerrs 6 // количество переменных, которое требуется считать
    bool err = false, errp[maxerrs]={false}; //переменные отображающие найденные ошибки: наличие ошибок вообще, и ошибки для каждой переменной
    QString errs; //строка для описания всех ошибок в ходе считывания
    QStringList errlist = {"- [коэффициент a]\n",
                          "- [коэффициент b]\n",
                          "- [коэффициент c]\n",
                          "- [левая граница]\n",
                          "- [правая граница]\n",
                          "- [число интервалов разбиения]\n"
                         }; //строки, содержащие описание переменных

    QString fileName = QFileDialog::getOpenFileName(this, QString("Open File"),
                                                    QString(""),
                                                    QString("TXT (*.txt);;All files (*.*)")); //выбор файла
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly)) //открытие файла
    {
       // Работаем с файлом.
        QString str = file.readAll(); //записываем файл в строку
        file.close(); //закрываем файл

        QStringList per = str.split(10); //режем большую строку считанную из файла,         "split(10)" - разрезане строки по символу с кодом
                                         //на маленькие строки содержащие переменные        10 (это символ переноса на следующую строку)

        if (per.size() != maxerrs) { //сравниваем количество строк с количеством переменных, которое требуется ввести
            if (per.size()>maxerrs) { //если строк больше чем нужно
                err=true;
                errs="Были считаны первые 6 строк.";
            }
            if (per.size()<maxerrs) { //если строк меньше чем нужно
                err=true;
                errs="В файле недостаточно данных.";
            }
        }

        for (int i=0;((i<per.size()) && (i<maxerrs)); i++) {   //проверяем переменные на успешное считывание. если считывание успешно, то записываем переменные в программу
            per.at(i).toDouble(&errp[i]); //промеряем можно ли перевести текющую строку в число
            if (errp[i]) { //если можно, то...
                switch (i) { //...записываем значение считанное из текущей строки в переменную
                case 0:
                {
                    ui->doubleSpinBox_koef_a->setValue(per.at(i).toDouble()); //коэффициент a
                    break;
                }
                case 1:
                {
                    ui->doubleSpinBox_koef_b->setValue(per.at(i).toDouble()); //коэффициент b
                    break;
                }
                case 2:
                {
                    ui->doubleSpinBox_koef_c->setValue(per.at(i).toDouble()); //коэффициент c
                    break;
                }
                case 3:
                {
                    ui->doubleSpinBox_pred_1->setValue(per.at(i).toDouble()); //левый предел
                    break;
                }
                case 4:
                {
                    ui->doubleSpinBox_pred_2->setValue(per.at(i).toDouble()); //правый предел
                    break;
                }
                case 5:
                {
                    ui->spinBox_interv->setValue(per.at(i).toDouble()); //число интервалов разбиения
                    break;
                }
                default:
                    break;
                }
            }
            ui->groupBox_input->setFocus();
        }

        //формирование отчета об ошибках

        if (err) {
            errs+="\n";
        }

        err=false;

        for (int i=0; i<maxerrs; i++) { //проверяем если хотя бы одна переменная не была считана
            err+=!(errp[i]);
        }

        if (err) {
            errs+="Переменная(ые)\n";
        }

        for (int i=0; i<maxerrs; i++) { //записываем, какие переменные не удалось считать
            if (!(errp[i])) {errs+=errlist[i];}
        }

        if (err) { //если в ходе считывания возникали ошибки, то выводим сообшение
            errs+="не была(и) считана(ы).";
            QMessageBox::critical(this, "Ошибка", errs);
        }
    }
}

void MainWindow::on_pushButton_file_hint_clicked()
{
    QMessageBox::about(this, "Ввод из файла", "Файл должен содержать данные, написанные\nчерез строку в следующем порядке:\n- [коэффициент a]\n- [коэффициент b]\n- [коэффициент c]\n- [левая граница]\n- [правая граница]\n- [число интервалов разбиения]\n Строки, содержащие посторонние символы считаны не будут.");

    ui->groupBox_input->setFocus();
}

void MainWindow::on_pushButton_about_clicked()
{
    if (QMessageBox::question(this, "О программе", "Данная программа была выполнена в среде разработки Qt Creator 4.8.2. Основной код: <a href=\"https://vk.com/id110732910\">Дмитрий Трофимов</a>, графический интерфейс <a href=\"https://vk.com/id200397338\">Дмитрий Митченков</a>. График выполнен с помощью виджета <a href=\"http://www.qcustomplot.com\">QCustomPlot</a>.", "Ok", "About QT")) {
        QMessageBox::aboutQt(this);
        }
    ui->groupBox_input->setFocus();
}

void MainWindow::on_doubleSpinBox_pred_1_valueChanged(double arg1)
{
    if(!(PRED_2>PRED_1) && !(PRED_2<PRED_1))
    {
        if(arg1 < 999) {
            ui->doubleSpinBox_pred_2->stepUp();
        }
        else {
            ui->doubleSpinBox_pred_2->setValue(PRED_2-1999);
        }
    }
}

void MainWindow::on_doubleSpinBox_pred_2_valueChanged(double arg2)
{
    if(!(PRED_2>PRED_1) && !(PRED_2<PRED_1))
    {
        if(arg2 > -999) {
            ui->doubleSpinBox_pred_1->stepDown();
        }
        else {
            ui->doubleSpinBox_pred_1->setValue(PRED_1+1999);
        }
    }
}

void MainWindow::on_pushButton_crack_pressed()
{
    ui->spinBox_interv->setRange(1,33554428); //предел для числа интервалов
    ui->horizontalSlider_interv->setRange(1,33554428); //дальше прога будет вылетать
    ui->horizontalSlider_interv->setTickInterval(3355443);
    ui->doubleSpinBox_koef_a->setRange(-INFINITY,INFINITY);
    ui->doubleSpinBox_koef_a->setDecimals(15);
    ui->doubleSpinBox_koef_b->setRange(-INFINITY,INFINITY);
    ui->doubleSpinBox_koef_b->setDecimals(15);
    ui->doubleSpinBox_koef_c->setRange(-INFINITY,INFINITY);
    ui->doubleSpinBox_koef_c->setDecimals(15);
    ui->doubleSpinBox_pred_1->setRange(-INFINITY,INFINITY);
    ui->doubleSpinBox_pred_1->setDecimals(15);
    ui->doubleSpinBox_pred_2->setRange(-INFINITY,INFINITY);
    ui->doubleSpinBox_pred_2->setDecimals(15);
    this->setWindowIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning)));
    ui->pushButton_crack->hide();
    QMessageBox::critical(this, "Снятие ограничений", "Дальнейшая стабильная работа программы не гарантируется.");
}

void MainWindow::keyPressEvent(QKeyEvent *event) //здесь можно задать различные действия выполняемые при нажатии клавиш
{
    switch (event->key()) {
    case Qt::Key_Return:
    {
        on_pushButton_calculate_clicked();
        break;
    }
    case Qt::Key_Left:
    {
        ui->comboBox_method->setCurrentIndex((ui->comboBox_method->currentIndex()+(ui->comboBox_method->maxCount()-1)) % ui->comboBox_method->maxCount());
        this->setWindowTitle(("Численное интегрирование методом "+ui->comboBox_method->currentText()));
        break;
    }
    case Qt::Key_Right:
    {
        ui->comboBox_method->setCurrentIndex((ui->comboBox_method->currentIndex()+1) % ui->comboBox_method->maxCount());
        this->setWindowTitle(("Численное интегрирование методом "+ui->comboBox_method->currentText()));
        break;
    }
//    case <код нажатой клавиши>:
//    {
//        <функция для клавиши>
//        break;
//    }
    default:
    {
//        QMessageBox::about(this,"Key",QString("%1").arg(event->key())); //если нужно узнать код нажатой клавиши
        break;
    }
    }
}
