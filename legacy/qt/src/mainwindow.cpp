#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QKeyEvent>
#include <QSettings>

#include <algorithm>
#include <iostream>


namespace
{
constexpr auto tag_background_color = "background-color";
constexpr auto tag_color_default = "COLOR_DEFAULT";
constexpr auto tag_color_hover = "COLOR_HOVER";
}


MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MainWindow)
{
   ui->setupUi(this);
   ui->statusbar->hide();
   ui->menubar->hide();

   auto file_reader = [](const std::string& filename) -> QString{
      QFile file(filename.c_str());
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
          return "";
      return file.readAll();
   };

   const auto title_label_css = file_reader("css/style_title_label.css");
   const auto title_buttons_css = file_reader("css/style_title_buttons.css");

   initWordStyle();

   ui->_title->setStyleSheet(title_label_css);
   ui->_close->setStyleSheet(title_buttons_css);
   ui->_maximize->setStyleSheet(title_buttons_css);
   ui->_minimize->setStyleSheet(title_buttons_css);

   setStyleSheet(file_reader("css/style_main.css"));

   setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

   QSettings settings("settings.ini", QSettings::IniFormat);
   ui->_flip->setChecked(settings.value("flipped").toBool());

   connect(
      ui->_close,
      &QPushButton::clicked,
      qApp,
      &QApplication::exit
   );

   connect(
      ui->_maximize,
      &QPushButton::clicked,
      this,
      &MainWindow::toggleMaximize
   );

   connect(
      ui->_minimize,
      &QPushButton::clicked,
      this,
      &MainWindow::showMinimized
   );


   setFocusPolicy(Qt::StrongFocus);

   initMenu();
   showMenu();

   qApp->installEventFilter(this);

   connect(
      ui->_start,
      &QPushButton::clicked,
      this,
      [this](){
         if (!ui->_files->currentItem())
         {
            return;
         }
         _test._flipped = ui->_flip->isChecked();
         _test.load(ui->_files->currentItem()->text().toStdString());
         showNext();
         ui->_stack->setCurrentIndex(0);
      }
   );

   connect(
      ui->_flip,
      &QCheckBox::stateChanged,
      this,
      [this](){
         QSettings settings("settings.ini", QSettings::IniFormat);
         settings.setValue("flipped", ui->_flip->isChecked());
      }
   );

   connect(
      ui->_pass,
      &QPushButton::clicked,
      this,
      [this](){
         pass();
      }
   );

   connect(
      ui->_fail,
      &QPushButton::clicked,
      this,
      [this](){
         fail();
      }
   );

   connect(
      ui->_word,
      &QPushButton::clicked,
      this,
      [this](){
         flip();
      }
   );
}


MainWindow::~MainWindow()
{
   delete ui;
}


bool MainWindow::eventFilter(QObject* /*obj*/, QEvent *event)
{
   if (ui->_stack->currentIndex() == 1)
   {
      return false;
   }

   if (event->type() == QEvent::KeyPress)
   {
      QKeyEvent* keyEvent = dynamic_cast<QKeyEvent *>(event);

      if (keyEvent->key() == Qt::Key_Left)
      {
         fail();
         return true;
      }
      else if (keyEvent->key() == Qt::Key_Right)
      {
         pass();
         return true;
      }
      else if (keyEvent->key() == Qt::Key_Space)
      {
         flip();
         return true;
      }
      else if (keyEvent->key() == Qt::Key_Escape)
      {
         showMenu();
         return true;
      }
   }

   return false;
}


void MainWindow::showNext()
{
   _flipped = false;
   const auto next_word = _test._words[_test._current];
   const auto next_word_lang_1 = next_word._language_1;
   ui->_word->setText(QString::fromStdString(next_word_lang_1).trimmed());

   // make those words that failed more often more red and angry
   updateWordStyle(next_word._score);
}


void MainWindow::showMenu()
{
   ui->_stack->setCurrentIndex(1);
}


void MainWindow::initMenu()
{
   const auto files = QDir::current().entryList({"*.csv"}, QDir::Filter::Files);
   ui->_files->insertItems(0, files);
   if (!files.empty())
   {
      ui->_files->setCurrentRow(0);
   }
}


void MainWindow::initWordStyle()
{
   QFile file("css/style_word.css");
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
   {
       return;
   }

   _word_stylesheet =  file.readAll();

   auto word_stylesheet_lines = _word_stylesheet.split("\n");

   auto readColor = [](const QString& line) {
      QRegularExpression rx("(#.*);");
      auto i = rx.globalMatch(line);
      const auto match = i.next();
      return QColor{match.captured(1)};
   };

   auto index = 0;
   for (auto& line : word_stylesheet_lines)
   {
      if (line.contains(tag_background_color))
      {
         if (index == 0)
         {
            _word_color_default = readColor(line);
            line = QString("%1: %2;").arg(tag_background_color).arg(tag_color_default);
         }
         else
         {
            _word_color_hover = readColor(line);
            line = QString("%1: %2;").arg(tag_background_color).arg(tag_color_hover);
         }

         index++;
      }
   }

   _word_stylesheet = word_stylesheet_lines.join("\n");
}


void MainWindow::updateWordStyle(int32_t score)
{
   // make bad ones red
   const auto fail_factor = std::min(std::max(-score, 0) * 0.1f, 1.0f);
   QColor good_color_default = _word_color_default;
   QColor good_color_hover = _word_color_hover;
   good_color_default.setRedF(std::min(1.0f, good_color_default.redF() + fail_factor));
   good_color_hover.setRedF(std::min(1.0f, good_color_hover.redF() + fail_factor));
   QString css = _word_stylesheet;
   css.replace(tag_color_default, good_color_default.name());
   css.replace(tag_color_hover, good_color_hover.name());
   ui->_word->setStyleSheet(css);
}


void MainWindow::fail()
{
   _test.fail();
   if (_test.next())
   {
      showNext();
   }
   else
   {
      showMenu();
   }
}


void MainWindow::pass()
{
   _test.pass();
   if (_test.next())
   {
      showNext();
   }
   else
   {
      showMenu();
   }
}


void MainWindow::flip()
{
   _flipped = !_flipped;

   auto translation =
         _flipped
            ? _test._words[_test._current]._language_2
            : _test._words[_test._current]._language_1;

   ui->_word->setText(QString::fromStdString(translation).trimmed());
}


void MainWindow::toggleMaximize()
{
   if (isMaximized())
   {
      showNormal();
      ui->_maximize->setText("☐");
   }
   else
   {
      showMaximized();
      ui->_maximize->setText("🗗");
   }
}


void MainWindow::mousePressEvent(QMouseEvent* event)
{
  _mouse_pos[0] = event->position().x();
  _mouse_pos[1] = event->position().y();
}


void MainWindow::mouseDoubleClickEvent(QMouseEvent* /*event*/)
{
   toggleMaximize();
}


void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
   move(
      event->globalPosition().x() - _mouse_pos[0],
      event->globalPosition().y() - _mouse_pos[1]
   );
}

