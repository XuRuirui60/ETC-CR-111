using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;//使用藍芽需引入

namespace _111
{
    public partial class Form1 : Form
    {
        SerialPort BT = new SerialPort();//建立BT藍芽序列傳輸Port
        CheckBox[] checkBoxes;
        public Form1()
        {
            InitializeComponent();
            groupBox3.Enabled = false;
            groupBox4.Enabled = false;
            button2.Enabled = false;
            comBoxSet();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            label3.Text = $"{DateTime.Now.ToString("yyyy/M/d HH:mm:ss")}";
        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void button7_Click(object sender, EventArgs e)
        {
            this.Close();
            Environment.Exit(Environment.ExitCode);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrWhiteSpace(comboBox1.Text.Trim()))
            {
                BT.PortName = comboBox1.Text;
                BT.BaudRate = 38400;//藍芽預設連接速度
                BT.WriteTimeout = 3000;//預設連接時間3秒
                BT.Open();//開啟藍芽Tx com port
                label2.BackColor = Color.Green;
                label2.Text = "Device Online";
                groupBox3.Enabled = true;
                groupBox4.Enabled = true;
                button2.Enabled = true;
                button1.Enabled = false;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            label2.BackColor = Color.Red;
            label2.Text = "Device Offline";
            groupBox3.Enabled = false;
            groupBox4.Enabled = false;
            button2.Enabled = false;
            button1.Enabled = true;
            BT.Close();
        }

        public void comBoxSet() //掃描現有的COM PORT加入COM選單
        {
            foreach (string s in SerialPort.GetPortNames())
            {
                comboBox1.Items.Add(s);
            }
        }

        private void groupBox6_Enter(object sender, EventArgs e)
        {

        }

        private void button5_Click(object sender, EventArgs e)
        {
            BT.Write("T");
        }

        private void button6_Click(object sender, EventArgs e)
        {
            BT.Write("P");
        }
        
        private void button4_Click(object sender, EventArgs e)
        {
            BT.Write("R");
            string inData = BT.ReadLine(); // 讀取一行數據
            textBox2.Text = inData;
        }

        private void groupBox4_Enter(object sender, EventArgs e)
        {
            
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (textBox1.Text.Length != 4)//只能輸入4個字
            {
                textBox1.Text = "";
            }
            else//否則清除
            {
                BT.Write("W" + textBox1.Text);
            }
        }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            Brush brush = new SolidBrush(Color.DarkRed);
            g.FillEllipse(brush, 10, 10, panel1.Width - 20, panel1.Height - 20);
        }

        private void panel2_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            Brush brush = new SolidBrush(Color.Olive);
            g.FillEllipse(brush, 10, 10, panel1.Width - 20, panel1.Height - 20);
        }

        private void panel3_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            Brush brush = new SolidBrush(Color.DarkGreen);
            g.FillEllipse(brush, 10, 10, panel1.Width - 20, panel1.Height - 20);
        }

        private void panel4_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            Brush brush = new SolidBrush(Color.DarkRed);
            g.FillEllipse(brush, 10, 10, panel1.Width - 20, panel1.Height - 20);
        }

        private void panel5_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            Brush brush = new SolidBrush(Color.Olive);
            g.FillEllipse(brush, 10, 10, panel1.Width - 20, panel1.Height - 20);
        }

        private void panel6_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            Brush brush = new SolidBrush(Color.DarkGreen);
            g.FillEllipse(brush, 10, 10, panel1.Width - 20, panel1.Height - 20);
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
