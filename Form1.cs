using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Lab1_Titov
{
    public partial class Form1 : Form
    {
        private int clientID = -1;
        private System.Windows.Forms.Timer sessionTimer;
        private System.Windows.Forms.Timer messageTimer;
        private int lastSessionCount = -1;
        private const int MT_DATA = 4;
        private const int MT_TIMEOUT_EXIT = 7;

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern int initClient(string name);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern void sendMessage(int to, int from, string message);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern int getSessionList(int clientID, [Out] char[] buffer, int bufferSize);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern int getMessage(int clientID, out int type, out int from, [Out] char[] buffer, int bufferSize);

        [DllImport(@"C:\Users\user\Documents\Lab1_Titov\x64\Debug\DLL_Titov.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void exitClient(int clientID);

        public Form1()
        {
            InitializeComponent();
            clientID = initClient("");

            this.Text = "Клиент " + clientID;  // Устанавливаем заголовок формы с ID клиента

            // Таймер для обновления списка сессий
            sessionTimer = new System.Windows.Forms.Timer();
            sessionTimer.Interval = 1000;
            sessionTimer.Tick += UpdateSessionList;
            sessionTimer.Start();

            // Таймер для получения сообщений
            messageTimer = new System.Windows.Forms.Timer();
            messageTimer.Interval = 1000; // Каждую секунду
            messageTimer.Tick += ReceiveMessages;
            messageTimer.Start();

            // Остановка таймера, когда курсор мыши находится в границах SessionListBox
            SessionListBox.MouseEnter += (s, e) => sessionTimer.Stop();
            SessionListBox.MouseLeave += (s, e) => sessionTimer.Start();

        }

        private void UpdateSessionList(object sender, EventArgs e)
        {
            char[] buffer = new char[1024];
            int len = getSessionList(clientID, buffer, 1024);
            if (len > 0)
            {
                string sessionData = new string(buffer, 0, len);
                string[] sessions = sessionData.Split(';');
                int currentCount = sessions.Length;

                // Обновляем список только если количество сессий изменилось
                if (currentCount != lastSessionCount)
                {
                    lastSessionCount = currentCount;
                    SessionListBox.BeginUpdate();
                    int selectedIndex = SessionListBox.SelectedIndex;
                    int topIndex = SessionListBox.TopIndex;

                    SessionListBox.Items.Clear();
                    SessionListBox.Items.Add("Все клиенты");
                    foreach (string session in sessions)
                    {
                        string[] parts = session.Split(':');
                        if (parts.Length == 2)
                        {
                            SessionListBox.Items.Add(parts[1]); // "Клиент <ID>"
                        }
                    }

                    // Восстанавливаем выбор и позицию прокрутки
                    if (selectedIndex >= 0 && selectedIndex < SessionListBox.Items.Count)
                        SessionListBox.SelectedIndex = selectedIndex;
                    if (topIndex < SessionListBox.Items.Count)
                        SessionListBox.TopIndex = topIndex;

                    SessionListBox.EndUpdate();
                }
            }
        }

        private void ReceiveMessages(object sender, EventArgs e)
        {
            try
            {
                char[] buffer = new char[1024];
                int type, from;
                int len = getMessage(clientID, out type, out from, buffer, 1024);
                if (len > 0 && type == MT_DATA)
                {
                    string message = new string(buffer, 0, len);
                    MessageListBox.Items.Add(message);
                }
                else if (type == MT_TIMEOUT_EXIT)
                {
                    this.Close(); // Закрываем форму при таймауте
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Исключение: " + ex.Message);
                this.Close();
            }
        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            if (SessionListBox.SelectedIndex == -1)
            {
                MessageBox.Show("Выберите получателя");
                return;
            }

            int to;
            string selected = SessionListBox.SelectedItem.ToString();
            if (selected == "Все клиенты")
            {
                to = -2; // Отправка всем
            }
            else
            {
                string idStr = selected.Split(' ')[1]; // Берем ID из "Клиент <ID>"
                to = int.Parse(idStr);
            }

            string message = RichTextBox.Text;
            sendMessage(to, clientID, message);
            RichTextBox.Clear();
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            exitClient(clientID); // Вызов функции завершения клиента
            sessionTimer?.Stop();
            messageTimer?.Stop();
            base.OnFormClosing(e);
        }
    }
}