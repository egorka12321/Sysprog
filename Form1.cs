using System;
using System.Net.Sockets;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
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
        private const string SERVER_IP = "127.0.0.1";
        private const int SERVER_PORT = 12345;

        [StructLayout(LayoutKind.Sequential)]
        public struct MessageHeader
        {
            public int to;
            public int from;
            public int type;
            public int size;
        }

        public enum MessageTypes
        {
            MT_INIT = 0,
            MT_EXIT = 1,
            MT_GETSESSIONS = 2,
            MT_GETDATA = 3,
            MT_DATA = 4,
            MT_NODATA = 5,
            MT_UPDATE = 6,
            MT_TIMEOUT_EXIT = 7
        }

        public Form1()
        {
            InitializeComponent();
            try
            {
                clientID = InitClient("");
                this.Text = "Клиент " + clientID;

                sessionTimer = new System.Windows.Forms.Timer();
                sessionTimer.Interval = 1000;
                sessionTimer.Tick += UpdateSessionList;
                sessionTimer.Start();

                messageTimer = new System.Windows.Forms.Timer();
                messageTimer.Interval = 1000;
                messageTimer.Tick += ReceiveMessages;
                messageTimer.Start();

                SessionListBox.MouseEnter += (s, e) => sessionTimer.Stop();
                SessionListBox.MouseLeave += (s, e) => sessionTimer.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка инициализации клиента: " + ex.Message);
                Application.Exit();
            }
        }

        private int InitClient(string name)
        {
            using (TcpClient client = new TcpClient(SERVER_IP, SERVER_PORT))
            using (NetworkStream stream = client.GetStream())
            {
                MessageHeader header = new MessageHeader
                {
                    to = 0,
                    from = 0,
                    type = (int)MessageTypes.MT_INIT,
                    size = name.Length * 2
                };

                byte[] headerBytes = StructToBytes(header);
                byte[] nameBytes = Encoding.Unicode.GetBytes(name);

                stream.Write(headerBytes, 0, headerBytes.Length);
                stream.Write(nameBytes, 0, nameBytes.Length);

                byte[] responseHeader = new byte[Marshal.SizeOf<MessageHeader>()];
                stream.Read(responseHeader, 0, responseHeader.Length);

                MessageHeader response = BytesToStruct<MessageHeader>(responseHeader);

                if (response.type == (int)MessageTypes.MT_INIT)
                {
                    byte[] nameBuffer = new byte[response.size];
                    stream.Read(nameBuffer, 0, nameBuffer.Length);
                    return response.to;
                }

                return -1;
            }
        }

        private void SendMessage(int to, int from, string message)
        {
            using (TcpClient client = new TcpClient(SERVER_IP, SERVER_PORT))
            using (NetworkStream stream = client.GetStream())
            {
                MessageHeader header = new MessageHeader
                {
                    to = to,
                    from = from,
                    type = (int)MessageTypes.MT_DATA,
                    size = message.Length * 2
                };

                byte[] headerBytes = StructToBytes(header);
                byte[] messageBytes = Encoding.Unicode.GetBytes(message);

                stream.Write(headerBytes, 0, headerBytes.Length);
                stream.Write(messageBytes, 0, messageBytes.Length);
            }
        }

        private string GetSessionList(int clientID)
        {
            using (TcpClient client = new TcpClient(SERVER_IP, SERVER_PORT))
            using (NetworkStream stream = client.GetStream())
            {
                MessageHeader request = new MessageHeader
                {
                    to = 0,
                    from = clientID,
                    type = (int)MessageTypes.MT_GETSESSIONS,
                    size = 0
                };

                byte[] requestBytes = StructToBytes(request);
                stream.Write(requestBytes, 0, requestBytes.Length);

                byte[] responseHeader = new byte[Marshal.SizeOf<MessageHeader>()];
                stream.Read(responseHeader, 0, responseHeader.Length);

                MessageHeader response = BytesToStruct<MessageHeader>(responseHeader);

                if (response.type == (int)MessageTypes.MT_DATA && response.size > 0)
                {
                    byte[] data = new byte[response.size];
                    stream.Read(data, 0, data.Length);
                    return Encoding.Unicode.GetString(data);
                }

                return "";
            }
        }

        private (int type, int from, string message) GetMessage(int clientID)
        {
            using (TcpClient client = new TcpClient(SERVER_IP, SERVER_PORT))
            using (NetworkStream stream = client.GetStream())
            {
                MessageHeader request = new MessageHeader
                {
                    to = 0,
                    from = clientID,
                    type = (int)MessageTypes.MT_GETDATA,
                    size = 0
                };

                byte[] requestBytes = StructToBytes(request);
                stream.Write(requestBytes, 0, requestBytes.Length);

                byte[] responseHeader = new byte[Marshal.SizeOf<MessageHeader>()];
                stream.Read(responseHeader, 0, responseHeader.Length);

                MessageHeader response = BytesToStruct<MessageHeader>(responseHeader);

                if (response.type == (int)MessageTypes.MT_DATA && response.size > 0)
                {
                    byte[] data = new byte[response.size];
                    stream.Read(data, 0, data.Length);
                    string msg = Encoding.Unicode.GetString(data);
                    return (response.type, response.from, msg);
                }
                else if (response.type == (int)MessageTypes.MT_TIMEOUT_EXIT)
                {
                    return (response.type, response.from, "");
                }

                return ((int)MessageTypes.MT_NODATA, -1, "");
            }
        }

        private void ExitClient(int clientID)
        {
            using (TcpClient client = new TcpClient(SERVER_IP, SERVER_PORT))
            using (NetworkStream stream = client.GetStream())
            {
                MessageHeader header = new MessageHeader
                {
                    to = 0,
                    from = clientID,
                    type = (int)MessageTypes.MT_EXIT,
                    size = 0
                };

                byte[] headerBytes = StructToBytes(header);
                stream.Write(headerBytes, 0, headerBytes.Length);
            }
        }

        private void UpdateSessionList(object sender, EventArgs e)
        {
            try
            {
                string sessionData = GetSessionList(clientID);
                if (!string.IsNullOrEmpty(sessionData))
                {
                    string[] sessions = sessionData.Split(';');
                    int currentCount = sessions.Length;

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
                                SessionListBox.Items.Add(parts[1]);
                            }
                        }

                        if (selectedIndex >= 0 && selectedIndex < SessionListBox.Items.Count)
                            SessionListBox.SelectedIndex = selectedIndex;
                        if (topIndex < SessionListBox.Items.Count)
                            SessionListBox.TopIndex = topIndex;

                        SessionListBox.EndUpdate();
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка обновления списка сессий: " + ex.Message);
            }
        }

        private void ReceiveMessages(object sender, EventArgs e)
        {
            try
            {
                var (type, from, message) = GetMessage(clientID);

                if (type == MT_DATA && !string.IsNullOrEmpty(message))
                {
                    MessageListBox.Items.Add($"От {from}: {message}");
                }
                else if (type == MT_TIMEOUT_EXIT)
                {
                    this.Close();
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
                to = -2;
            }
            else
            {
                string idStr = selected.Split(' ')[1];
                if (!int.TryParse(idStr, out to))
                {
                    MessageBox.Show("Неверный формат ID получателя");
                    return;
                }
            }

            string message = RichTextBox.Text;
            if (string.IsNullOrEmpty(message)) return;

            try
            {
                SendMessage(to, clientID, message);
                RichTextBox.Clear();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка отправки сообщения: " + ex.Message);
            }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            try
            {
                ExitClient(clientID);
            }
            catch { }
            finally
            {
                sessionTimer?.Stop();
                messageTimer?.Stop();
                base.OnFormClosing(e);
            }
        }

        // Helper methods
        public static byte[] StructToBytes<T>(T obj) where T : struct
        {
            int size = Marshal.SizeOf<T>();
            IntPtr ptr = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr(obj, ptr, false);
                byte[] bytes = new byte[size];
                Marshal.Copy(ptr, bytes, 0, size);
                return bytes;
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }
        }

        public static T BytesToStruct<T>(byte[] data) where T : struct
        {
            IntPtr ptr = Marshal.AllocHGlobal(data.Length);
            try
            {
                Marshal.Copy(data, 0, ptr, data.Length);
                return (T)Marshal.PtrToStructure(ptr, typeof(T));
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }
        }
    }
}