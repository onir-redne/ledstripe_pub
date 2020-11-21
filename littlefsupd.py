# update and upload littleFS
import sys
import argparse
import os
import subprocess

ESPLIB_PATH_FSTOOLS = 'tools/mklittlefs/2.5.0-4-fe5bb56/'
ESPLIB_PATH_HWTOOLS = 'hardware/esp8266/2.7.4/tools/esptool/'
ESPLIB_PATH = 'C:/Users/onir/AppData/Local/Arduino15/packages/esp8266/'

class ExceptionLfsUpd(Exception):
    def __init__(self, message):
        super().__init__(message)

def get_platform():
    platforms = {
        'linux1' : 'Linux',
        'linux2' : 'Linux',
        'darwin' : 'OS X',
        'win32' : 'Windows'
    }
    if sys.platform not in platforms:
        return sys.platform

    return platforms[sys.platform]

class LittleFSMgr(object):

    def __init__(self, esplib_path, lf_size, lf_output, lf_port, lf_baud='921600', lf_page_size=256, lf_block_size=8192, lf_flash_size=1024**2 * 4):
        self.esp_path = esplib_path
        self.size = lf_size
        self.page_size = lf_page_size
        self.block_size = lf_block_size
        self.data_path = ''
        self.output = lf_output
        self.port = lf_port
        self.baud = lf_baud
        self.upload_address = None
        self.esptool = 'esptool.py'
        self.python3 = 'python'
        platform = get_platform()

        if self.size not in [1024**2, 1024**2 * 2, 1024**2 * 3]:
            raise ExceptionLfsUpd('Invalid Filesystem size, must be 1, 2 or 3 MB!')

        self.upload_address = lf_flash_size - self.size
        print('upload address: 0x{0:08X}'.format(self.upload_address))
        self.size = self.size - 1024 * 24
        print('fixing fs size to: {0} B'.format(self.size ))
        

        if platform == 'Linux' or platform == 'OS X':
            self.mklittlefs = 'mklittlefs'            
            self.data_path = './data'
        elif platform == 'Windows':
            self.mklittlefs = 'mklittlefs.exe'
            self.data_path = 'data'
        else:
            raise ExceptionLfsUpd('unknown platform!')

        self.mklittlefs_path = os.path.join(self.esp_path, ESPLIB_PATH_FSTOOLS, self.mklittlefs)
        if not os.path.getmtime(self.mklittlefs_path):
            raise ExceptionLfsUpd('missing littleFS tool: ' + self.mklittlefs_path)

        self.esptool_path = os.path.join(self.esp_path, ESPLIB_PATH_HWTOOLS, self.esptool)
        if not os.path.getmtime(self.mklittlefs_path):
            raise ExceptionLfsUpd('missing littleFS tool: ' + self.mklittlefs_path)

    def build_fs(self):
        print('Building LittleFS:\nsize: {0}\npage: {1}\nblock: {2}\n'.format(self.size, self.page_size, self.block_size))
        print(self.mklittlefs_path + ' -c ' + str(self.data_path) + ' -p ' +  str(self.page_size) + ' -b ' + str(self.block_size) + ' -s ' + str(self.size))
        process = subprocess.Popen([self.mklittlefs_path, '-c ' + str(self.data_path), '-p ' +  str(self.page_size), '-b ' + str(self.block_size), '-s ' + str(self.size), self.output])
        if process.wait() != 0:
            raise ExceptionLfsUpd('error crating littleFS')

        print('LittleFS created!')

    def upload_fs(self):
        # uploadCmd, "--chip", "esp8266", "--port", serialPort, "--baud", uploadSpeed, "write_flash", uploadAddress, imagePath
        # esptool.py --port COM4 --baud 921600 --before default_reset write_flash 0x100000 ledstripe.img
        print('Uploading littleFS image file: {0} -> {1} / {2}'.format(self.output, self.baud, self.port))
        #process = subprocess.Popen([self.python3 , self.esptool_path, '--port', 'COM4', '--baud', '921600', '--before', 'default_reset', 'write_flash', '0x100000', 'ledstripe.img'])
        process = subprocess.Popen([self.python3 , self.esptool_path, '--port', self.port,'--baud', self.baud, '--before', 'default_reset', '--after', 'hard_reset', 'write_flash',  hex(self.upload_address), self.output])
        if process.wait() != 0:
            raise ExceptionLfsUpd('error crating littleFS')

        print('LittleFS created!')


if __name__ == "__main__":

    lfs = LittleFSMgr(ESPLIB_PATH, 1024**2*3, 'build/ledstripe.img', 'COM4')
    lfs.build_fs()
    lfs.upload_fs()

