import wave
from pydub import AudioSegment
AudioSegment.ffmpeg = r"C:\ffmpeg\bin\ffmpeg.exe"
import numpy as np
import io

from PyQt5.QtWidgets import QApplication, QMainWindow, QFileDialog
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QEvent, QIODevice, QDir, QFileInfo
from PyQt5.QtGui import QIcon
from unicodedata import normalize

from interface import Ui_MainWindow

class SoundApp(QMainWindow):
    CHUNK_SIZE = 4096

    def __init__(self):
        super().__init__()
        self.audio_serial = QSerialPort()
        self.control_serial = QSerialPort()
        self.duration = 1
        self.current_chunk = 0
        self.sample_rate = 1
        self.num_frames = 1
        self.volume = 50
        self.bit_depth = 1
        self.selected_file = ''
        self.num_channels = 1
        self.wav_file = None
        self.ui = Ui_MainWindow()
        self.init_main_window_params()
        self.update_com_combobox()

        self.init_volume()
        self.init_events()
        self.init_connects()


    # def get_sample_rate(self):
    #     return self.sample_rate
    #
    # def get_duration(self):
    #     return self.duration
    #
    # def get_num_frames(self):
    #     return self.num_frames
    #
    # def get_selected_file(self):
    #     return self.selected_file

    def closeEvent(self, a0):
        self.stop()

    def eventFilter(self, source, event):
        if source == self.ui.COM_Audio_ComboBox or source == self.ui.COM_Control_ComboBox:
            if event.type() == QEvent.MouseButtonPress:
                self.update_com_combobox()
        return super().eventFilter(source, event)

    def init_main_window_params(self):
        self.ui.setupUi(self)
        self.setWindowTitle('Simple SOUND CARDISHE')
        self.setWindowIcon(QIcon('icon.webp'))

        self.gui_control_disable()

    def init_connects(self):
        self.ui.COM_Audio_PushButton.clicked.connect(self.open_audio_com_port)
        self.ui.COM_Control_PushButton.clicked.connect(self.open_control_com_port)
        self.ui.Choose_File_PushButton.clicked.connect(self.open_file_dialog)
        self.ui.Audio_Slider.valueChanged.connect(self.update_position)
        self.ui.Volume_Slider.valueChanged.connect(self.update_volume)
        self.ui.play_PushButton.clicked.connect(self.play)
        self.ui.Stop_PushButton.clicked.connect(self.stop)

    def init_events(self):
        self.ui.COM_Audio_ComboBox.installEventFilter(self)
        self.ui.COM_Control_ComboBox.installEventFilter(self)

    def init_volume(self):
        self.ui.Volume_Slider.setMinimum(0)
        self.ui.Volume_Slider.setMaximum(100)
        self.ui.Volume_Slider.setValue(self.volume)

    def open_audio_com_port(self):
        self.audio_serial.setPortName(self.ui.COM_Audio_ComboBox.currentText().split(' ')[0])
        self.audio_serial.setBaudRate(921600)
        if self.audio_serial.open(QIODevice.ReadWrite):
            self.ui.COM_Audio_PushButton.setDisabled(True)
            self.ui.COM_Audio_ComboBox.setDisabled(True)

    def gui_control_disable(self):
        """disable control gui elements on the main window"""
        self.ui.Audio_Slider.setDisabled(True)
        self.ui.play_PushButton.setDisabled(True)
        self.ui.Stop_PushButton.setDisabled(True)

    def gui_control_enable(self):
        """enable control gui elements on the main window"""
        self.ui.Audio_Slider.setEnabled(True)
        self.ui.play_PushButton.setEnabled(True)
        self.ui.Stop_PushButton.setEnabled(True)

    def open_control_com_port(self):
        self.control_serial.setPortName(self.ui.COM_Control_ComboBox.currentText().split(' ')[0])
        self.control_serial.setBaudRate(115200)
        if self.control_serial.open(QIODevice.ReadWrite):
            self.ui.COM_Control_PushButton.setDisabled(True)
            self.ui.COM_Control_ComboBox.setDisabled(True)

    def update_com_combobox(self):
        """method updates COM port combo boxes"""
        selected_port = self.audio_serial.portName() + self.control_serial.portName()
        ports = self._get_available_ports()
        for port in ports:
            if port.split(' ')[0] == selected_port:
                ports.remove(port)
        if self.ui.COM_Audio_ComboBox.isEnabled():
            self.ui.COM_Audio_ComboBox.clear()
            self.ui.COM_Audio_ComboBox.addItems(ports)
        if self.ui.COM_Control_ComboBox.isEnabled():
            self.ui.COM_Control_ComboBox.clear()
            self.ui.COM_Control_ComboBox.addItems(ports)

    def _get_available_ports(self):
        """get available COM ports in format 'port name + manufacturer'"""
        port_list = []
        ports = QSerialPortInfo().availablePorts()
        for port in ports:
            port_list.append(port.portName() + ' ' + port.manufacturer())
        return port_list

    def update_position(self):
        """Update time label based on slider value"""
        self.current_chunk = self.ui.Audio_Slider.value()
        self.update_time_label(self.current_chunk)

    def update_volume(self):
        self.volume = self.ui.Volume_Slider.value()

    def open_file_dialog(self):
        """Open file choosing dialog and setups other ui modules according to the file"""
        file_dialog = QFileDialog(self)
        file_dialog.setNameFilter("Audio Files (*.wav *.mp3 *.flac)")
        file_dialog.setDirectory(QDir.rootPath())
        if file_dialog.exec_():
            self.selected_file = file_dialog.selectedFiles()[0]
            self.setup_according_to_file()
            self.update_time_label(0)
            self.gui_control_enable()

    def update_time_label(self, current_frame):
        current_time = current_frame / self.sample_rate
        cur_sec = int(current_time % 60)
        cur_min = int(current_time // 60)
        dur_sec = int(self.duration % 60)
        dur_min = int(self.duration // 60)
        self.ui.Time_Label.setText(f'{cur_min:02}.{cur_sec:02}/{dur_min:02}.{dur_sec:02}')

    def setup_according_to_file(self):
        self.ui.File_Name_Label.setText(QFileInfo(self.selected_file).fileName())
        self.read_file()
        self.ui.Audio_Slider.setMinimum(0)
        self.ui.Audio_Slider.setMaximum(self.num_frames)

    def read_file(self):
        """reads wav file and sets duration, number of frames and sample rate"""
        try:
            audio = AudioSegment.from_file(self.selected_file)
            audio = audio.set_channels(1).set_sample_width(1)
            output = io.BytesIO()
            audio.export(output, format="wav")
            output.seek(0)

            self.wav_file = wave.open(output, 'rb')
            self.sample_rate = self.wav_file.getframerate()
            self.num_frames = self.wav_file.getnframes()
            self.duration = self.num_frames / self.sample_rate
            self.bit_depth = self.wav_file.getsampwidth()
            self.num_channels = self.wav_file.getnchannels()
            self.audio_serial.readyRead.connect(self.on_ready_read)
        except Exception as e:
            print(f'error: {e}')

    def on_ready_read(self):
        """Sends next chunk when there is '1' in audio serial"""
        data = self.audio_serial.readAll()
        #print(data)
        self.wav_file.setpos(self.current_chunk)
        if data == b'1': # check audio serial for prompt for new chunk
            chunk = self.wav_file.readframes(self.CHUNK_SIZE)
            if chunk:
                #chunk = self.normalize_to_uint8(chunk)
                chunk = self.adjust_volume(chunk)
                self.audio_serial.write(chunk)
                self.ui.Audio_Slider.setValue(self.current_chunk + self.CHUNK_SIZE)
            else:
                self.stop()
                self.ui.Audio_Slider.setValue(0)



    def adjust_volume(self, chunk):
        chunk_int = np.frombuffer(chunk, dtype=np.uint8)
        chunk_int = chunk_int.astype(np.int16)
        chunk_int = chunk_int - 128
        chunk_int = chunk_int * self.volume / 50
        chunk_int = np.clip(chunk_int, -127, 127)
        adjusted_chunk = (chunk_int + 128).astype(np.uint8)
        return adjusted_chunk.tobytes()

    def play(self):
        self.control_serial.write(b'1')

    def stop(self):
        self.control_serial.write(b'3')



if __name__ == "__main__":
    app = QApplication([])
    window = SoundApp()
    window.show()
    app.exec_()