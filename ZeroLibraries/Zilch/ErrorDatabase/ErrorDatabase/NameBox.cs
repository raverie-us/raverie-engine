using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ErrorDatabase
{
    public partial class NameBox : Form
    {
        public NameBox()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.Hide();
        }

        private void NameText_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 13)
                button1_Click(sender, e);

            if (char.IsLetter(e.KeyChar) == false && e.KeyChar != '\b')
                e.KeyChar = '\0';
        }

        private void NameText_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
