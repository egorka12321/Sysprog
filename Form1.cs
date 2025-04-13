using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.Timers;
using System.Windows.Forms;

namespace Lab1_Titov
{
    public partial class Form1 : Form
    {
        private Process process;
        private int threadCountStop = 0;
        private System.Windows.Forms.Timer updateTimer;

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CharSet = CharSet.Unicode)]
        private static extern void sendCommand(int selected_thread, int commandId, string message);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CharSet = CharSet.Unicode)]
        public static extern IntPtr getLastServerResponse();

        public Form1()
        {
            InitializeComponent();
            UpdateStopButtonState();
            UpdateThreadsListBox();
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            // Очищаем список потоков
            ListBox.Items.Clear();

            // Если работает внешний процесс, можно отслеживать событие его завершения, как в вашем варианте
            if (process != null)
            {
                process.EnableRaisingEvents = true;
                process.Exited += (s, ev) =>
                {
                    this.Invoke(new Action(() => ListBox.Items.Clear()));
                };
            }

            // Добавляем базовые записи
            ListBox.Items.Add("Главный поток");
            ListBox.Items.Add("Все потоки");

            int existingThreads = ListBox.Items.Count - 2;
            int newThreads = (int)NumericUpDown.Value;

            // Создаем новые потоки, отправляя команду через DLL (commandId = 0 означает создание)
            for (int i = 0; i < newThreads; i++)
            {
                sendCommand(existingThreads + i, 0, "Создать поток");
                ListBox.Items.Add($"Поток {existingThreads + i + threadCountStop}");
            }

            UpdateStopButtonState();
            UpdateThreadsListBox();
        }
        private void StopButton_Click(object sender, EventArgs e)
        {
            if (ListBox.Items.Count <= 2)
            {
                sendCommand(-1, 1, "Завершить все");
            }
            else
            {
                // Остановка последнего созданного потока
                int threadId = ListBox.Items.Count - 3;
                sendCommand(threadId, 1, "Завершить поток");
                ListBox.Items.RemoveAt(ListBox.Items.Count - 1);
            }

            UpdateStopButtonState();
            UpdateThreadsListBox();
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

            int selectedThread = -2;
            string sel = ListBox.SelectedItem.ToString();

            // Если выбран "Главный поток" или "Все потоки"
            if (sel == "Главный поток")
                selectedThread = -1;
            else if (sel == "Все потоки")
                selectedThread = -2; // можно трактовать как посылку всем потокам
            else
            {
                // Разбираем номер потока из строки, например "Поток 3"
                string[] parts = sel.Split(' ');
                if (parts.Length >= 2 && int.TryParse(parts[1], out int threadNum))
                {
                    selectedThread = threadNum;
                }
            }

            // Отправляем команду с commandId = 2 для отправки сообщения
            sendCommand(selectedThread, 2, RichTextBox.Text);

        }

        private void UpdateThreadsListBox()
        {
            // Отправляем команду запроса списка потоков (commandId = 3)
            sendCommand(-1, 3, "");
            IntPtr responsePtr = getLastServerResponse();
            string response = Marshal.PtrToStringUni(responsePtr);
            UpdateListBoxFromResponse(response);
        }

        private void OnTimeout(Object source, ElapsedEventArgs e)
        {
            UpdateThreadsListBox();
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            var timer = new System.Timers.Timer(1000);
            timer.Elapsed += OnTimeout;
            timer.AutoReset = true;
            timer.Enabled = true;
        }

        private void UpdateListBoxFromResponse(string response)
        {
            if (InvokeRequired)
            {
                Invoke(new Action(() => UpdateListBoxFromResponse(response)));
                return;
            }

            ListBox.Items.Clear();
            ListBox.Items.Add("Главный поток");
            ListBox.Items.Add("Все потоки");

            // Ожидается, что ответ имеет формат, где после запятой идут номера потоков (например: ",0,1,2")
            string[] parts = response.Split(',');
            if (parts.Length > 1)
            {
                for (int i = 1; i < parts.Length; i++)
                {
                    if (int.TryParse(parts[i], out int id))
                    {
                        ListBox.Items.Add($"Поток {id}");
                    }
                }
            }
        }
    }
}
