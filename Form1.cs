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
        private static extern void SendData(int selected_thread, string text);

        System.Threading.EventWaitHandle eventStart = new EventWaitHandle(false, EventResetMode.AutoReset, "eventStart");
        System.Threading.EventWaitHandle eventConfirm = new EventWaitHandle(false, EventResetMode.AutoReset, "eventConfirm");
        System.Threading.EventWaitHandle eventStop = new EventWaitHandle(false, EventResetMode.AutoReset, "eventStop");
        System.Threading.EventWaitHandle eventExit = new EventWaitHandle(false, EventResetMode.AutoReset, "eventExit");
        System.Threading.EventWaitHandle eventSend = new EventWaitHandle(false, EventResetMode.AutoReset, "eventSend");
        public Form1()
        {
            InitializeComponent();
            this.FormClosing += Form1_FormClosing;
            UpdateStopButtonState();
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            if (Process.GetProcessesByName("Titov_c").Length == 0)
            {
                ListBox.Items.Clear();
                process = Process.Start("C:\\Users\\user/Documents\\Lab1_Titov\\x64/Debug\\Titov_c.exe");

                process.EnableRaisingEvents = true;
                process.Exited += (s, ev) =>
                {
                    this.Invoke(new Action(() => ListBox.Items.Clear()));
                };

                ListBox.Items.Add("Главный поток");
                ListBox.Items.Add("Все потоки");
                UpdateStopButtonState();
            }
            else
            {
                if (NumericUpDown.Value <= 0)
                {
                    MessageBox.Show("Укажите корректное количество потоков.");
                    return;
                }

                int existingThreads = ListBox.Items.Count - 2;
                for (int i = existingThreads + 1; i <= existingThreads + NumericUpDown.Value; i++)
                {
                    eventStart.Set();
                    eventConfirm.WaitOne();
                    ListBox.Items.Add($"Поток: {i + threadCountStop}");
                }
            }
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            eventStop.Set();

            if (ListBox.Items.Count != 2)
            {
                ListBox.Items.RemoveAt(ListBox.Items.Count - 1);
                threadCountStop += 1;
            }
            else
            {
                ListBox.Items.Clear();
            }
            UpdateStopButtonState();
        }
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            eventExit.Set();
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
                eventSend.Set();
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
