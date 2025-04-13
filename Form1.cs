using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;

namespace Lab1_Titov
{
    public partial class Form1 : Form
    {
        private Process process;
        private int threadCountStop = 0;

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CharSet = CharSet.Unicode)]
        private static extern void connectClient(int selected_thread, string text);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CharSet = CharSet.Unicode)]
        private static extern void sendCommand(int selected_thread, int commandId, string message);

        public Form1()
        {
            InitializeComponent();
            this.FormClosing += Form1_FormClosing;
            UpdateStopButtonState();
        }

        private void StartButton_Click(object sender, EventArgs e)
        {

            ListBox.Items.Clear();

            process.EnableRaisingEvents = true;
            process.Exited += (s, ev) =>
            {
                this.Invoke(new Action(() => ListBox.Items.Clear()));
            };

            ListBox.Items.Add("Главный поток");
            ListBox.Items.Add("Все потоки");
            UpdateStopButtonState();

            int existingThreads = ListBox.Items.Count - 2;
            for (int i = existingThreads + 1; i <= existingThreads + NumericUpDown.Value; i++)
            {
                ListBox.Items.Add($"Поток: {i + threadCountStop}");
            }
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            if (ListBox.Items.Count <= 2)
            {
                sendCommand(-1, 1, "Завершить все");
            }
            else
            {
                sendCommand(threadId, 1, "Завершить поток");
                ListBox.Items.Clear();
            }
            UpdateStopButtonState();
        }

        private void UpdateStopButtonState()
        {
            StopButton.Enabled = ListBox.Items.Count > 0;
            SendButton.Enabled = ListBox.Items.Count > 0;
        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            if (ListBox.SelectedIndex < 0)
            {
                MessageBox.Show("Выберите поток");
                return;
            }
            int selectedThread = ListBox.SelectedIndex - 1;
            if (selectedThread > 0)
            {
                string message = RichTextBox.Text;
                SendData(selectedThread + threadCountStop, message);
            }
            else
            {
                string message = RichTextBox.Text;
                SendData(selectedThread, message);
                eventSend.Set();
            }

        }
    }
}
