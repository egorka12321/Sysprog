using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Lab1_Titov
{
    public partial class Form1 : Form
    {
        private Process process;
        private int threadCountStop = 0;

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CharSet = CharSet.Unicode)]
        private static extern void sendCommand(int selected_thread, int commandId, string message);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CharSet = CharSet.Unicode)]
        public static extern int getLastServerResponse();

        public Form1()
        {
            InitializeComponent();
            UpdateThreadsListBox();
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            ListBox.Items.Add("Главный поток");
            ListBox.Items.Add("Все потоки");

            int existingThreads = ListBox.Items.Count - 2;
            int newThreads = (int)NumericUpDown.Value;

            for (int i = 0; i < newThreads; i++)
            {
                sendCommand(existingThreads + i, 0, "Создать поток");
                ListBox.Items.Add($"Поток {existingThreads + i + threadCountStop}");
            }

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
                sendCommand(ListBox.Items.Count - 3, 1, "Завершить поток");
                ListBox.Items.RemoveAt(ListBox.Items.Count - 1);
            }

            UpdateThreadsListBox();
        }


        private void SendButton_Click(object sender, EventArgs e)
        {
            if (ListBox.SelectedIndex < 0)
            {
                MessageBox.Show("Выберите поток");
                return;
            }
            sendCommand(ListBox.SelectedIndex - 2, 2, RichTextBox.Text);

        }

        private void UpdateThreadsListBox()
        {
            sendCommand(-1, 3, "");
            int count = getLastServerResponse();

            if (InvokeRequired)
            {
                Invoke(new Action(() => UpdateThreadsListBox()));
                return;
            }

            ListBox.Items.Clear();
            ListBox.Items.Add("Главный поток");
            ListBox.Items.Add("Все потоки");
            for (int i = 0; i < count; i++)
            {
                ListBox.Items.Add($"Поток {i}");
            }
        }
    }
}
