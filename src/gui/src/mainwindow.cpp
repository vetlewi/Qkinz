#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ame2012_masses.h"
#include "tablemakerhtml.h"
#include "BatchReader.h"

#include <iostream>
#include <cmath>

#include <QWebEngineView>
#include <QFile>
#include <QFileDialog>

const double PI = acos(-1);

static QString Unit2String(const Unit_t &unit)
{
    if (unit == mgcm2)
        return "mg/cm^2";
    else if (unit == gcm2)
        return "g/cm2";
    else
        return "um";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , setBeam_form(new SelectBeamForm(this))
    , setTarget_form(new SelectTargetForm(this))
    , setFrontBack_form(new SelectFrontBackForm(this))
    , setTelescope_form(new SelectTelescopeForm(this))
    , runDialog(new RunDialog(this))
{
    ui->setupUi(this);

    worker = new Worker(&theBeam, &theTarget, &theFront, &theBack, &theTelescope);
    //worker->setCustomTarget(new CustomPower("SKrC2D4_table2_ug.txt"), new CustomPower("SpC2D4_pstar_ug.txt"));
    worker->moveToThread(&workThread);

    bReader = new BatchReader();
    bReader->moveToThread(&batchThread);
    //ui->pushButton->setHidden(true);

    qRegisterMetaType<QVector<double> >("QVector<double>");
    qRegisterMetaType<Fragment_t>("Fragment_t");
    connect(&workThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainWindow::operate, worker, &Worker::Run);
    connect(worker, &Worker::ResultCurve, this, &MainWindow::CurveData);
    connect(worker, &Worker::ResultScatter, this, &MainWindow::ScatterData);
    connect(worker, &Worker::FinishedAll, this, &MainWindow::WorkFinished);
    connect(worker, &Worker::curr_prog, runDialog, &RunDialog::progress);
    workThread.start();

    qRegisterMetaType<QString>("QString");
    connect(&batchThread, &QThread::finished, bReader, &QObject::deleteLater);
    connect(this, &MainWindow::runBatchFile, bReader, &BatchReader::Start);
    connect(bReader, &BatchReader::FinishedAll, this, &MainWindow::finishBFile);
    connect(bReader, &BatchReader::curr_prog, runDialog, &RunDialog::progress);
    batchThread.start();

    ui->plotTab->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                 QCP::iSelectLegend | QCP::iSelectPlottables);

    ui->plotTab->xAxis->setLabel("E [MeV]");
    ui->plotTab->yAxis->setLabel("dE [MeV]");
    ui->plotTab->axisRect()->setupFullAxesBox();
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->plotTab->legend->setVisible(true);
    ui->plotTab->legend->setFont(legendFont);
    ui->plotTab->legend->setSelectedFont(legendFont);
    ui->plotTab->legend->setSelectableParts(QCPLegend::spItems);

    connect(ui->plotTab, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    connect(ui->plotTab, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->plotTab, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    connect(ui->plotTab->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotTab->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plotTab->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plotTab->yAxis2, SLOT(setRange(QCPRange)));


    connect(ui->ChangeBeamButton, SIGNAL(clicked()), this, SLOT(showBeam()));
    connect(ui->ChangeTargetButton, SIGNAL(clicked()), this, SLOT(showTarget()));
    connect(ui->ChangeFBButton, SIGNAL(clicked()), this, SLOT(showFB()));
    connect(ui->ChangeTelescopeButton, SIGNAL(clicked()), this, SLOT(showTelescope()));

    connect(ui->RunButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(ui->ResetButton, SIGNAL(clicked()), this, SLOT(Reset_All()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(BatchFile()));

    connect(setBeam_form, SIGNAL(DoRefresh()), this, SLOT(Refresh()));
    connect(setTarget_form, SIGNAL(DoRefresh()), this, SLOT(Refresh()));
    connect(setFrontBack_form, SIGNAL(DoRefresh()), this, SLOT(Refresh()));
    connect(setTelescope_form, SIGNAL(DoRefresh()), this, SLOT(Refresh()));

    connect(ui->actionClose, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(ui->StripNumbr, SIGNAL(valueChanged(int)), this, SLOT(strip_changed(int)));
    connect(ui->FwdAngRButton, SIGNAL(toggled(bool)), this, SLOT(toggle_Angle(bool)));
    connect(ui->BwdAngRButton, SIGNAL(toggled(bool)), this, SLOT(toggle_Angle(bool)));
    connect(ui->manAngle, SIGNAL(toggled(bool)), this, SLOT(toggle_Angle(bool)));

    connect(ui->otherFrag, SIGNAL(toggled(bool)), this, SLOT(toggle_frag(bool)));
    ui->otherA->setEnabled(false);
    ui->otherZ->setEnabled(false);

    // Setting default values. Should be done from saved file?
    theBeam.A = 1;
    theBeam.Z = 1;
    theBeam.E = 16.0;

    theTarget.A = 28;
    theTarget.Z = 14;
    theTarget.width = 4.0;
    theTarget.unit = mgcm2;

    theFront.is_present = false;
    theFront.A = 27;
    theFront.Z = 13;
    theFront.width = 0.5;
    theFront.unit = mgcm2;

    theBack.is_present = false;
    theBack.A = 27;
    theBack.Z = 13;
    theBack.width = 0.5;
    theBack.unit = mgcm2;

    theTelescope.dEdetector.Z = 14;
    theTelescope.dEdetector.width = 130.0;
    theTelescope.dEdetector.unit = um;
    theTelescope.Edetector.Z = 14;
    theTelescope.Edetector.width = 1550.0;
    theTelescope.Edetector.unit = um;
    theTelescope.has_absorber = true;
    theTelescope.Absorber.Z = 13;
    theTelescope.Absorber.width = 10.5;
    theTelescope.Absorber.unit = um;

    ui->StripNumbr->setEnabled(true);
    ui->manAngleInput->setEnabled(false);
    ui->incAngleInput->setEnabled(false);
    double incAngle = ui->StripNumbr->value() * 2 - 7;
    double angle = 47 + incAngle;
    if ( ui->BwdAngRButton->isChecked() )
        angle = 180. - angle;
    ui->manAngleInput->setValue(angle);
    ui->incAngleInput->setValue(fabs(incAngle));

    Refresh();
}

void MainWindow::Reset_All()
{
    theBeam.A = 1;
    theBeam.Z = 1;
    theBeam.E = 16.0;

    theTarget.A = 28;
    theTarget.Z = 14;
    theTarget.width = 4.0;
    theTarget.unit = mgcm2;

    theFront.is_present = false;
    theFront.A = 27;
    theFront.Z = 13;
    theFront.width = 0.5;
    theFront.unit = mgcm2;

    theBack.is_present = false;
    theBack.A = 27;
    theBack.Z = 13;
    theBack.width = 0.5;
    theBack.unit = mgcm2;

    theTelescope.dEdetector.Z = 14;
    theTelescope.dEdetector.width = 130.0;
    theTelescope.dEdetector.unit = um;
    theTelescope.Edetector.Z = 14;
    theTelescope.Edetector.width = 1550.0;
    theTelescope.Edetector.unit = um;
    theTelescope.has_absorber = true;
    theTelescope.Absorber.Z = 13;
    theTelescope.Absorber.width = 10.5;
    theTelescope.Absorber.unit = um;
    ui->StripNumbr->setEnabled(true);
    ui->manAngleInput->setEnabled(false);
    ui->incAngleInput->setEnabled(false);

    double incAngle = ui->StripNumbr->value() * 2 - 7;
    double angle = 47 + incAngle;
    if ( ui->BwdAngRButton->isChecked() )
        angle = 180. - angle;
    ui->manAngleInput->setValue(angle);
    ui->incAngleInput->setValue(fabs(incAngle));

    RemoveAllGraphs();
    Refresh();
}


MainWindow::~MainWindow()
{
    delete ui;
    delete setBeam_form;
    delete setTarget_form;
    delete setTelescope_form;
    workThread.quit();
    workThread.wait();
    batchThread.quit();
    batchThread.wait();
}


void MainWindow::Refresh()
{
    std::string bName;
    if (theBeam.Z == 1){
        if (theBeam.A == 1)
            bName = "p";
        else if (theBeam.A == 2)
            bName = "d";
        else if (theBeam.A == 3)
            bName = "t";
        else
            bName = "?";
    } else if (theBeam.Z == 2 && theBeam.A == 2){
        bName = "\u03B1";
    } else {
        bName = std::string(get_element_name(theBeam.Z)) + "-" + std::to_string(theBeam.A);
    }
    std::string bEnergy = std::to_string(theBeam.E);
    ui->CurrentBeam->setText(bName.c_str());
    ui->CurrentEnergy->setText(bEnergy.c_str());

    std::string tName = std::to_string(theTarget.A) + std::string(get_element_name(theTarget.Z));
    std::string tWidth = std::to_string(theTarget.width);
    ui->CurrentTarget->setText(tName.c_str());
    ui->CurrentThickness->setText(tWidth.c_str());
    ui->TargetUnitLabel->setText(Unit2String(theTarget.unit));


    if (theFront.is_present){
        ui->CurrentFront->setText(std::string(std::to_string(theFront.A) + std::string(get_element_name(theFront.Z))).c_str());
        ui->FrontWidth->setText(std::to_string(theFront.width).c_str());
        ui->FrontUnitLabel->setText(Unit2String(theFront.unit));
    } else {
        ui->CurrentFront->setText("-");
        ui->FrontWidth->setText("-");
    }
    if (theBack.is_present){
        ui->CurrentBack->setText(std::string(std::to_string(theBack.A) + std::string(get_element_name(theBack.Z))).c_str());
        ui->BackWidth->setText(std::to_string(theBack.width).c_str());
        ui->BackUnitLabel->setText(Unit2String(theBack.unit));
    } else {
        ui->CurrentBack->setText("-");
        ui->BackWidth->setText("-");
    }

    ui->dEMatSym->setText(get_element_name(theTelescope.dEdetector.Z));
    ui->CurrentDE->setText(std::to_string(theTelescope.dEdetector.width).c_str());
    ui->dEdetUnitLabel->setText(Unit2String(theTelescope.dEdetector.unit));
    ui->EMatSym->setText(get_element_name(theTelescope.Edetector.Z));
    ui->CurrentE->setText(std::to_string(theTelescope.Edetector.width).c_str());
    ui->EdetUnitLabel->setText(Unit2String(theTelescope.Edetector.unit));
    if (theTelescope.has_absorber){
        ui->AMatSym->setText(get_element_name(theTelescope.Absorber.Z));
        ui->CurrentA->setText(std::to_string(theTelescope.Absorber.width).c_str());
        ui->AbsUnitLabel->setText(Unit2String(theTelescope.Absorber.unit));
    } else {
        ui->AMatSym->setText("-");
        ui->CurrentA->setText("-");
    }

}

void MainWindow::CurveData(const QVector<double> &ex, const QVector<double> &x, const QVector<double> &y, const QVector<double> &coeff, const Fragment_t &what)
{
    QString legend = QString("%1(%2,").arg(ui->CurrentTarget->text()).arg(ui->CurrentBeam->text());
    if (what == Proton){
        int Zres = theBeam.Z + theTarget.Z - 1;
        int Ares = theBeam.A + theTarget.A - 1;
        QString outP = std::string(std::to_string(Ares) + get_element_name(Zres)).c_str();
        if (theBeam.A == 1 && theBeam.Z == 1){
            legend = QString("%1%2)%3").arg(legend).arg("p'").arg(outP);
        } else {
            legend = QString("%1%2)%3").arg(legend).arg("p").arg(outP);
        }
        ui->plotTab->addPlottable(makeCurve(x, y, QPen(QColor(255,0,0)), legend));
        table.setCoeff(coeff, TableMakerHTML::Proton);
        table.setCurve(ex, x, y, TableMakerHTML::Proton);
    } else if (what == Deutron){
        int Zres = theBeam.Z + theTarget.Z - 1;
        int Ares = theBeam.A + theTarget.A - 2;
        QString outP = std::string(std::to_string(Ares) + get_element_name(Zres)).c_str();
        if (theBeam.A == 2 && theBeam.Z == 1){
            legend = QString("%1%2)%3").arg(legend).arg("d'").arg(outP);
        } else {
            legend = QString("%1%2)%3").arg(legend).arg("d").arg(outP);
        }
        ui->plotTab->addPlottable(makeCurve(x, y, QPen(QColor(0,255,0)), legend));
        table.setCoeff(coeff, TableMakerHTML::Deutron);
        table.setCurve(ex, x, y, TableMakerHTML::Deutron);
    } else if (what == Triton){
        int Zres = theBeam.Z + theTarget.Z - 1;
        int Ares = theBeam.A + theTarget.A - 3;
        QString outP = std::string(std::to_string(Ares) + get_element_name(Zres)).c_str();
        if (theBeam.A == 3 && theBeam.Z == 1){
            legend = QString("%1%2)%3").arg(legend).arg("t'").arg(outP);
        } else {
            legend = QString("%1%2)%3").arg(legend).arg("t").arg(outP);
        }
        ui->plotTab->addPlottable(makeCurve(x, y, QPen(QColor(128,128,128)), legend));
        table.setCoeff(coeff, TableMakerHTML::Triton);
        table.setCurve(ex, x, y, TableMakerHTML::Triton);
    } else if (what == Helium3){
        int Zres = theBeam.Z + theTarget.Z - 2;
        int Ares = theBeam.A + theTarget.A - 3;
        QString outP = std::string(std::to_string(Ares) + get_element_name(Zres)).c_str();
        if (theBeam.A == 3 && theBeam.Z == 2){
            legend = QString("%1%2)%3").arg(legend).arg("He-3'").arg(outP);
        } else {
            legend = QString("%1%2)%3").arg(legend).arg("He-3").arg(outP);
        }
        ui->plotTab->addPlottable(makeCurve(x, y, QPen(QColor(0,0,255)), legend));
        table.setCoeff(coeff, TableMakerHTML::Helium3);
        table.setCurve(ex, x, y, TableMakerHTML::Helium3);
    } else if (what == Alpha){
        int Zres = theBeam.Z + theTarget.Z - 2;
        int Ares = theBeam.A + theTarget.A - 4;
        QString outP = std::string(std::to_string(Ares) + get_element_name(Zres)).c_str();
        if (theBeam.A == 4 && theBeam.Z == 2){
            legend = QString("%1%2)%3").arg(legend).arg("α'").arg(outP);
        } else {
            legend = QString("%1%2)%3").arg(legend).arg("α").arg(outP);
        }
        ui->plotTab->addPlottable(makeCurve(x, y, QPen(QColor(0,0,0)), legend));
        table.setCoeff(coeff, TableMakerHTML::Alpha);
        table.setCurve(ex, x, y, TableMakerHTML::Alpha);
    } else if (what == Other){
        int Zres = theBeam.Z + theTarget.Z - ui->otherZ->value();
        int Ares = theBeam.A + theTarget.A - ui->otherA->value();
        QString outP = std::string(std::to_string(Ares) + get_element_name(Zres)).c_str();
        QString inP = std::string(std::to_string(ui->otherA->value()) + get_element_name(ui->otherZ->value())).c_str();
        if ( theBeam.A == ui->otherA->value() && theBeam.Z == ui->otherZ->value() ){
            legend = QString("%1%2)%3").arg(legend).arg(inP+"'").arg(outP);
        } else {
            legend = QString("%1%2)%3").arg(legend).arg(inP).arg(outP);
        }
        ui->plotTab->addPlottable(makeCurve(x, y, QPen(QColor(128, 128, 0)), legend));
        table.setCoeff(coeff, TableMakerHTML::Other);
        table.setCurve(ex, x, y, TableMakerHTML::Other);
    }

    ui->plotTab->replot();
    ui->plotTab->rescaleAxes();
    ui->plotTab->show();
}

void MainWindow::ScatterData(const QVector<double> &x, const QVector<double> &dx, const QVector<double> &y,
                             const QVector<double> &dy, const QVector<double> &ex, const Fragment_t &what)
{
    if (what == Proton){
        ui->plotTab->addPlottable(makeGraph(x, dx, y, dy, QPen(QColor(255,0,0))));
        ui->plotTab->plottable()->removeFromLegend();
        table.setData(ex, y, dy, x, dx, TableMakerHTML::Proton);
    } else if (what == Deutron){
        ui->plotTab->addPlottable(makeGraph(x, dx, y, dy, QPen(QColor(0,255,0))));
        ui->plotTab->plottable()->removeFromLegend();
        table.setData(ex, y, dy, x, dx, TableMakerHTML::Deutron);
    } else if (what == Triton){
        ui->plotTab->addPlottable(makeGraph(x, dx, y, dy, QPen(QColor(128,128,128))));
        ui->plotTab->plottable()->removeFromLegend();
        table.setData(ex, y, dy, x, dx, TableMakerHTML::Triton);
    } else if (what == Helium3){
        ui->plotTab->addPlottable(makeGraph(x, dx, y, dy, QPen(QColor(0,0,255))));
        ui->plotTab->plottable()->removeFromLegend();
        table.setData(ex, y, dy, x, dx, TableMakerHTML::Helium3);
    } else if (what == Alpha){
        ui->plotTab->addPlottable(makeGraph(x, dx, y, dy, QPen(QColor(0,0,0))));
        ui->plotTab->plottable()->removeFromLegend();
        table.setData(ex, y, dy, x, dx, TableMakerHTML::Alpha);
    } else if (what == Other){
        ui->plotTab->addPlottable(makeGraph(x, dx, y, dy, QPen(QColor(128, 128, 0))));
        ui->plotTab->plottable()->removeFromLegend();
        table.setData(ex, y, dy, x, dx, TableMakerHTML::Other);
    }
    ui->plotTab->replot();
    ui->plotTab->rescaleAxes();
    ui->plotTab->show();
}

void MainWindow::run()
{
    runDialog->restart_counter();
    runDialog->show();

    RemoveAllGraphs();
    table.Reset();
    double angle;
    double incAngle;

    if (ui->manAngle->isChecked()){
        angle = ui->manAngleInput->value()*PI/180.;
        incAngle = ui->incAngleInput->value()*PI/180.;
    } else {
        incAngle = (2*ui->StripNumbr->value() - 7)*PI/180.;
        angle = 47.0*PI/180. + incAngle;
        if (ui->BwdAngRButton->isChecked()){
            angle = PI - angle;
        }
    }
    int A = -1;
    int Z = -1;
    if ( ui->otherFrag->isChecked() ){
        A = ui->otherA->value();
        Z = ui->otherZ->value();
    }
    emit operate(angle, incAngle, ui->protons->isChecked(), ui->deutrons->isChecked(), ui->tritons->isChecked(), ui->He3s->isChecked(), ui->alphas->isChecked(), A, Z);
}

void MainWindow::WorkFinished()
{
    ui->webView->setHtml(table.getHTMLCode());
    emit runDialog->Finished();
}

void MainWindow::selectionChanged()
{
    if (ui->plotTab->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotTab->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        ui->plotTab->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotTab->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)){
        ui->plotTab->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->plotTab->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    if (ui->plotTab->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotTab->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        ui->plotTab->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->plotTab->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels)){
        ui->plotTab->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->plotTab->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
}

void MainWindow::mousePress()
{
    if (ui->plotTab->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->plotTab->axisRect()->setRangeDrag(ui->plotTab->xAxis->orientation());
    else if (ui->plotTab->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->plotTab->axisRect()->setRangeDrag(ui->plotTab->yAxis->orientation());
    else
        ui->plotTab->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
    if (ui->plotTab->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
       ui->plotTab->axisRect()->setRangeZoom(ui->plotTab->xAxis->orientation());
     else if (ui->plotTab->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
       ui->plotTab->axisRect()->setRangeZoom(ui->plotTab->yAxis->orientation());
     else
       ui->plotTab->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::RemoveAllGraphs()
{
    ui->plotTab->clearPlottables();
    ui->plotTab->replot();
}

QCPCurve *MainWindow::makeCurve(QVector<double> x, QVector<double> y, QPen pen, QString label)
{
    QCPCurve *nCurve = new QCPCurve(ui->plotTab->xAxis, ui->plotTab->yAxis);

    nCurve->setName(label);
    nCurve->setPen(pen);
    nCurve->setData(x, y);
    nCurve->setLineStyle(QCPCurve::lsLine);

    return nCurve;
}

QCPGraph *MainWindow::makeGraph(QVector<double> x, QVector<double> dx, QVector<double> y, QVector<double> dy, QPen pen)
{
    QCPGraph *nGraph = new QCPGraph(ui->plotTab->xAxis, ui->plotTab->yAxis);

    nGraph->removeFromLegend();
    nGraph->setDataBothError(x, y, dx, dy);
    nGraph->setPen(pen);
    nGraph->setLineStyle(QCPGraph::lsNone);
    nGraph->setScatterStyle(QCPScatterStyle::ssDisc);
    nGraph->setErrorType(QCPGraph::etBoth);

    return nGraph;
}

void MainWindow::on_actionExport_plot_triggered()
{
    QString Filters("PDF (*.pdf);;Image (*.png);;Image (*.jpg);;Bitmap image (*.bmp)");
    QString DefaultFilter("PDF (*.pdf)");
    QFileDialog *SaveFigDialog = new QFileDialog(this);

    QString FilePath = SaveFigDialog->getSaveFileName(this, "Save figure", QDir::homePath(), Filters, &DefaultFilter);
    if (FilePath.endsWith(".pdf", Qt::CaseInsensitive))
        ui->plotTab->savePdf(FilePath);
    else if (FilePath.endsWith(".png", Qt::CaseInsensitive))
        ui->plotTab->savePng(FilePath);
    else if (FilePath.endsWith(".jpg", Qt::CaseInsensitive))
        ui->plotTab->saveJpg(FilePath);
    else if (FilePath.endsWith(".bmp", Qt::CaseInsensitive))
        ui->plotTab->saveBmp(FilePath);
    delete SaveFigDialog;
}

void MainWindow::on_actionExport_table_triggered()
{
    QString Filters("PDF (*.pdf);;HTML (*.html);;Text file (*.txt)");
    QString DefaultFilter("PDF (*.pdf)");
    QFileDialog *SaveTabDialog = new QFileDialog(this);

    QString FilePath = SaveTabDialog->getSaveFileName(this, "Save table", QDir::homePath(), Filters, &DefaultFilter);
    if (FilePath.endsWith("pdf", Qt::CaseInsensitive)){
        ui->webView->page()->printToPdf(FilePath);
    } else if (FilePath.endsWith("html", Qt::CaseInsensitive)){
        QString htmlCode = table.getHTMLCode();
        QFile file(FilePath);
        if (file.open(QIODevice::WriteOnly|QIODevice::Text)){
            QTextStream out(&file);
            out << htmlCode;
        }
        file.close();
    } else if (FilePath.endsWith("txt", Qt::CaseInsensitive)){
        QString txt = table.getTXT();
        QFile file(FilePath);
        if (file.open(QIODevice::WriteOnly|QIODevice::Text)){
            QTextStream out(&file);
            out << txt;
        }
        file.close();
    }

    delete SaveTabDialog;
}

void MainWindow::on_actionSave_triggered()
{
    QString Filters("Qkinz file (.qkz)");
    QFileDialog *SaveSettingDialog = new QFileDialog(this);

    QString FilePath = SaveSettingDialog->getSaveFileName(this, "Save setup", QDir::homePath(), Filters, &Filters);
    if (!FilePath.isEmpty()){
        if (!FilePath.endsWith(".qkz"))
            FilePath += ".qkz";
        FILE *outFile = std::fopen(FilePath.toStdString().c_str(), "wb");
        fwrite(&theBeam, sizeof(Beam_t), 1, outFile);
        fwrite(&theTarget, sizeof(Target_t), 1, outFile);
        fwrite(&theTelescope, sizeof(Telescope_t), 1, outFile);
        std::fclose(outFile);
    }

    delete SaveSettingDialog;
}

void MainWindow::on_actionOpen_triggered()
{
    QString Filters("Qkinz file (.qkz)");
    QFileDialog *OpenSettingDialog = new QFileDialog(this);

    QString FilePath = OpenSettingDialog->getOpenFileName(this, "Open setup", QDir::homePath());//, Filters, &Filters);
    if (!FilePath.isEmpty()){
        FILE *inFile = std::fopen(FilePath.toStdString().c_str(), "rb");
        fread(&theBeam, sizeof(Beam_t), 1, inFile);
        fread(&theTarget, sizeof(Target_t), 1, inFile);
        fread(&theTelescope, sizeof(Telescope_t), 1, inFile);
        std::fclose(inFile);
    }

    Refresh();
    delete OpenSettingDialog;
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionAbout_triggered()
{
    QFile file(":/text/about.html");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream ts(&file);
    ts.setCodec("UTF-8");

    QMessageBox::about(this, tr("About Qkinz"), ts.readAll());
    file.close();

}

void MainWindow::on_actionAbout_QCustomPlot_triggered()
{
    QFile file(":/text/about_qcustomplot.html");
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream ts(&file);
    ts.setCodec("UTF-8");
    QMessageBox::about(this, tr("About QCustomPlot 1.30"), ts.readAll());
    file.close();
}

void MainWindow::strip_changed(int)
{
    double incAngle = 2*ui->StripNumbr->value() - 7;
    double angle = 47.0 + incAngle;
    if ( ui->BwdAngRButton->isChecked() ){
        angle = 180. - angle;
    }
    ui->manAngleInput->setValue(angle);
    ui->incAngleInput->setValue(fabs(incAngle));
}

void MainWindow::toggle_Angle(bool)
{
    if (ui->manAngle->isChecked()){
        ui->StripNumbr->setDisabled(true);
        ui->manAngleInput->setEnabled(true);
        ui->incAngleInput->setEnabled(true);
    } else {
        ui->StripNumbr->setEnabled(true);
        ui->manAngleInput->setEnabled(false);
        ui->incAngleInput->setEnabled(false);
    }
    if (ui->BwdAngRButton->isChecked()){
        strip_changed(0);
    }

}

void MainWindow::toggle_frag(bool)
{
    if ( ui->otherFrag->isChecked() ){
        ui->otherA->setEnabled(true);
        ui->otherZ->setEnabled(true);
    } else {
        ui->otherA->setEnabled(false);
        ui->otherZ->setEnabled(false);
    }
}

// This is an early implementation.
// There are no garantee that the software is correct
// and this is especialy true for this functionality.
void MainWindow::BatchFile()
{
    batchFiles.clear();
    currentBatch = 0;
    QFileDialog *openBatchDialog = new QFileDialog(this);
    batchFiles = openBatchDialog->getOpenFileNames(this, "Choose batchfile", QDir::homePath());


    if (!batchFiles.isEmpty()){
        runDialog->restart_counter();
        runDialog->show();
        emit runBatchFile(batchFiles.at(0));
        currentBatch += 1;
    }
}

void MainWindow::finishBFile()
{
    if ((currentBatch < batchFiles.length())){
        emit runBatchFile(batchFiles.at(currentBatch));
        currentBatch += 1;
        runDialog->restart_counter();
    } else {
        WorkFinished();
    }
}
