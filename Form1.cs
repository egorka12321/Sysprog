using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Timers;

namespace Lab1_Titov
{
    public partial class Form1 : Form
    {
        private Process process;
        private int threadCountStop = 0;
        private System.Timers.Timer updateTimer; // Используем System.Timers.Timer

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CharSet = CharSet.Unicode)]
        private static extern void sendCommand(int commandId, int selected_thread, string message);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll")]
        public static extern int getLastServerResponse();

        public Form1()
        {
            InitializeComponent();

            // Настройка и запуск таймера
            updateTimer = new System.Timers.Timer(1000); // интервал 1 секунда
            updateTimer.Elapsed += UpdateTimer_Elapsed;
            updateTimer.AutoReset = true;
            updateTimer.Enabled = true;

            // Отключать таймер при взаимодействии с ListBox, чтобы не сбрасывать выбор
            ListBox.Enter += (s, e) => updateTimer.Stop();
            ListBox.Leave += (s, e) => updateTimer.Start();

            UpdateThreadsListBox();
        }

        private void UpdateTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            if (InvokeRequired)
            {
                Invoke((Action)UpdateThreadsListBox);
            }
            else
            {
                UpdateThreadsListBox();
            }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (updateTimer != null)
            {
                updateTimer.Stop();
                updateTimer.Dispose();
            }
            base.OnFormClosing(e);
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            int existingThreads = ListBox.Items.Count - 2;
            int newThreads = (int)NumericUpDown.Value;

            for (int i = 0; i < newThreads; i++)
            {
                sendCommand(0, existingThreads + i, "Создать поток");
            }

            UpdateThreadsListBox();
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            if (ListBox.Items.Count <= 2)
            {
                sendCommand(1, -1, "Завершить все");
            }
            else
            {
                int lastIndex = ListBox.Items.Count - 1;
                sendCommand(1, lastIndex - 2, "Завершить поток");
            }

            UpdateThreadsListBox();
        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            if (ListBox.SelectedIndex < 0)
            {
                MessageBox.Show("Выберите конкретный поток или 'Все потоки'");
                return;
            }

            string message = RichTextBox.Text;
            int selectedThread = ListBox.SelectedIndex - 2;
            sendCommand(2, selectedThread, message);
        }

        private void UpdateThreadsListBox()
        {
            sendCommand(3, -1, string.Empty);
            int count = getLastServerResponse();

            ListBox.BeginUpdate();
            int selectedIndex = ListBox.SelectedIndex;
            int topIndex = ListBox.TopIndex;

            ListBox.Items.Clear();
            ListBox.Items.Add("Главный поток");
            ListBox.Items.Add("Все потоки");
            for (int i = 0; i < count; i++)
            {
                ListBox.Items.Add($"Поток {i}");
            }

            // Восстанавливаем выбор и позицию прокрутки
            if (selectedIndex >= 0 && selectedIndex < ListBox.Items.Count)
                ListBox.SelectedIndex = selectedIndex;
            if (topIndex < ListBox.Items.Count)
                ListBox.TopIndex = topIndex;

            ListBox.EndUpdate();
        }
    }
}