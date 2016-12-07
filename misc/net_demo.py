#******************************************************************
#
# Institute for System Programming of the Russian Academy of Sciences
# Copyright (C) 2016 ISPRAS
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, Version 3.
#
# This program is distributed in the hope # that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License version 3 for more details.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

import sys
import time
import socket
from PyQt5.QtCore import pyqtSignal, QObject, QThread
from PyQt5.QtWidgets import QWidget, QMainWindow, QApplication, QProgressBar, QPushButton, QLCDNumber, QVBoxLayout, QHBoxLayout, QLabel
from PyQt5.QtGui import QFont

from time import sleep

SIZE = 1024
MAX_LEN = 4096
class Net(QThread):
    perf_data = pyqtSignal(str, name="changed") #New style signal

    def __init__(self, parent):
        print("INIT")
        QThread.__init__(self,parent)
        self.stop_flag = False

    def run(self):

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(('192.168.56.1', 10003))
        i = 0
        time_sum = 0
        #count = 128
        count = 90
        times = 100
        sum_val = 0

        j = 0
        while 1:
            j+=1
            if self.stop_flag:
                break
            s.sendto(bytes([count]) + bytes(SIZE - 1), ('192.168.56.101', 10001))

            start = time.time()
            for i in range(count):
                print("receiving", i)
                data = s.recv(4096)
                if data == "":
                    print("no packet !!!");
                print("recv packet ", data[0])
            stop = time.time()

            val = SIZE*count/(stop - start)/1024/1024
            #print(val)
            sum_val += val
            f = round(sum_val/(j+1), 2)

            print(f)
            self.perf_data.emit("{:.2f}".format(f))


        self.stop_flag = True

        print("Connection closed.")
        s.close()



class Crono(QThread):
    tick = pyqtSignal(int, name="changed") #New style signal

    def __init__(self, parent):
        print("INIT")
        QThread.__init__(self,parent)
        self.stop_flag = False

    def run(self):
        print("check status")
        for x in range(1,101):
            if self.stop_flag:
                break
            self.tick.emit(x)
            time.sleep(0.1)

class Widget(QWidget):

    def __init__(self):
        super(Widget, self).__init__()
        self.lcd = QLCDNumber(self)
        self.lcd.setDigitCount(8)
        self.started = False

        self.startClicked()

        btn_start = QPushButton("Run", self)
        btn_stop = QPushButton("Stop", self)

        btn_start.clicked.connect(self.startClicked)
        btn_stop.clicked.connect(self.stopClicked)

        lbl1 = QLabel('MBps', self)
        lbl1.setFont(QFont('SansSerif', 20))

        hbox = QHBoxLayout()
        hbox.addWidget(self.lcd)
        hbox.addWidget(lbl1)



        hbox2 = QHBoxLayout()
        hbox2.addWidget(btn_start)
        hbox2.addWidget(btn_stop)

        vbox = QVBoxLayout()
        vbox.addLayout(hbox)
        vbox.addLayout(hbox2)
        self.setLayout(vbox)

        self.setGeometry(300, 300, 450, 250)
        self.show()


    def startClicked(self):
        #self.crono = Crono(self)
        #self.crono.start()
        #Connect signal of self.crono to a slot of self.progressBar
        #self.crono.tick.connect(self.prgBar.setValue)
        #self.crono.tick.connect(self.lcd.display)
        if (not self.started):
            self.net = Net(self)
            self.net.start()
            self.net.perf_data.connect(self.lcd.display)
            self.started = True
        #self.net = Net(self)
        #self.net.start()

    def stopClicked(self):
        self.net.stop_flag = True
        self.net.quit()
        self.net.wait()
        self.started = False


if __name__ == '__main__':

    app = QApplication(sys.argv)
    ex = Widget()
    sys.exit(app.exec_())
