namespace Lab1_Titov
{
    partial class Form1
    {
        /// <summary>
        /// Обязательная переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором форм Windows

        /// <summary>
        /// Требуемый метод для поддержки конструктора — не изменяйте 
        /// содержимое этого метода с помощью редактора кода.
        /// </summary>
        private void InitializeComponent()
        {
            this.SessionListBox = new System.Windows.Forms.ListBox();
            this.SendButton = new System.Windows.Forms.Button();
            this.RichTextBox = new System.Windows.Forms.RichTextBox();
            this.MessageListBox = new System.Windows.Forms.ListBox();
            this.SuspendLayout();
            // 
            // SessionListBox
            // 
            this.SessionListBox.FormattingEnabled = true;
            this.SessionListBox.ItemHeight = 16;
            this.SessionListBox.Location = new System.Drawing.Point(42, 38);
            this.SessionListBox.Name = "SessionListBox";
            this.SessionListBox.Size = new System.Drawing.Size(286, 356);
            this.SessionListBox.TabIndex = 5;
            // 
            // SendButton
            // 
            this.SendButton.Location = new System.Drawing.Point(409, 182);
            this.SendButton.Name = "SendButton";
            this.SendButton.Size = new System.Drawing.Size(138, 49);
            this.SendButton.TabIndex = 7;
            this.SendButton.Text = "Send";
            this.SendButton.UseVisualStyleBackColor = true;
            this.SendButton.Click += new System.EventHandler(this.SendButton_Click);
            // 
            // RichTextBox
            // 
            this.RichTextBox.Location = new System.Drawing.Point(362, 248);
            this.RichTextBox.Name = "RichTextBox";
            this.RichTextBox.Size = new System.Drawing.Size(235, 96);
            this.RichTextBox.TabIndex = 8;
            this.RichTextBox.Text = "";
            // 
            // MessageListBox
            // 
            this.MessageListBox.FormattingEnabled = true;
            this.MessageListBox.ItemHeight = 16;
            this.MessageListBox.Location = new System.Drawing.Point(631, 182);
            this.MessageListBox.Name = "MessageListBox";
            this.MessageListBox.Size = new System.Drawing.Size(276, 212);
            this.MessageListBox.TabIndex = 9;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(947, 413);
            this.Controls.Add(this.MessageListBox);
            this.Controls.Add(this.RichTextBox);
            this.Controls.Add(this.SendButton);
            this.Controls.Add(this.SessionListBox);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.ListBox SessionListBox;
        private System.Windows.Forms.Button SendButton;
        private System.Windows.Forms.RichTextBox RichTextBox;
        private System.Windows.Forms.ListBox MessageListBox;
    }
}

