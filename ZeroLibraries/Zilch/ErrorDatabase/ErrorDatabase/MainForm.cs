using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;
using System.Globalization;

namespace ErrorDatabase
{
    public partial class MainForm : Form
    {
        [Serializable]
        class ErrorExample
        {
            public String ErrorCode = String.Empty;
            public String FixedCode = String.Empty;
            public String ExplanationOfFix = String.Empty;
        }

        [Serializable]
        class ErrorInfo
        {
            public String Name = String.Empty;
            public String Error = String.Empty;
            public String Reason = String.Empty;

            public List<ErrorExample> Examples = new List<ErrorExample>();
        }

        ErrorInfo CurrentErrorInfo = null;

        ErrorExample CurrentExample = null;

        List<ErrorInfo> ErrorInfoList = new List<ErrorInfo>();

        BinaryFormatter Formatter = new BinaryFormatter();

        public MainForm()
        {
            InitializeComponent();
        }

        private void Add_Click(object sender, EventArgs e)
        {
            NameBox nameBox = new NameBox();
            nameBox.ShowDialog();

            string name = nameBox.NameText.Text.Replace(" ", "");
            ErrorList.Items.Add(name);

            ErrorInfoList.Add(new ErrorInfo() { Name = name });

            ErrorList.SelectedIndex = ErrorList.Items.Count - 1;
        }

        private void ErrorList_SelectedIndexChanged(object sender, EventArgs e)
        {
            this.ExampleList.Items.Clear();
            ExampleList_SelectedIndexChanged(sender, e);


            if (ErrorList.SelectedIndex != -1)
            {
                ErrorInfo info = ErrorInfoList[ErrorList.SelectedIndex];

                this.CurrentErrorInfo = null;

                this.ErrorGroup.Enabled = true;

                this.NameText.Text = info.Name;
                this.Error.Text = info.Error;
                this.Reason.Text = info.Reason;

                this.CurrentErrorInfo = info;

                int exampleCounter = 1;
                foreach (ErrorExample example in info.Examples)
                {
                    this.ExampleList.Items.Add(exampleCounter);
                    ++exampleCounter;
                }
            }
            else
            {
                this.CurrentErrorInfo = null;

                this.ErrorGroup.Enabled = false;

                this.NameText.Text = "";
                this.Error.Text = "";
                this.Reason.Text = "";
                this.ErrorCode.Text = "";
            }
        }

        private void Remove_Click(object sender, EventArgs e)
        {
            if (ErrorList.SelectedIndex != -1)
            {
                int temp = ErrorList.SelectedIndex;
                ErrorList.Items.RemoveAt(ErrorList.SelectedIndex);

                ErrorInfoList.RemoveAt(temp);
                ErrorList.SelectedIndex = temp - 1;
            }
        }

        private void NameText_TextChanged(object sender, EventArgs e)
        {
            if (CurrentErrorInfo != null)
            {
                int selStart = this.NameText.SelectionStart;
                int selLen = this.NameText.SelectionLength;
                CurrentErrorInfo.Name = this.NameText.Text;
                this.ErrorList.Items[ErrorList.SelectedIndex] = this.NameText.Text;
                this.NameText.Focus();

                this.NameText.SelectionStart = selStart;
                this.NameText.SelectionLength = selLen;
            }
        }

        private void Error_TextChanged(object sender, EventArgs e)
        {
            if (CurrentErrorInfo != null)
            {
                CurrentErrorInfo.Error = this.Error.Text;
            }
        }

        private void Reason_TextChanged(object sender, EventArgs e)
        {
            if (CurrentErrorInfo != null)
            {
                CurrentErrorInfo.Reason = this.Reason.Text;
            }
        }

        private void NameText_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (char.IsLetter(e.KeyChar) == false && e.KeyChar != '\b')
                e.KeyChar = '\0';
        }

        private void Save_Click(object sender, EventArgs e)
        {
            File.Copy(@"..\..\..\Errors.database", @"..\..\..\Errors.database.backup", true);
            FileStream stream = new FileStream(@"..\..\..\Errors.database", FileMode.Create);
            Formatter.Serialize(stream, this.ErrorInfoList);
            stream.Close();
        }

        private void RefreshAndSortLists()
        {
            // Sort the errors by name
            this.ErrorInfoList.Sort((a, b) =>
            {
                return a.Name.CompareTo(b.Name);
            });

            foreach (ErrorInfo info in this.ErrorInfoList)
            {
                ErrorList.Items.Add(info.Name);
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            try
            {
                FileStream stream = new FileStream(@"..\..\..\Errors.database", FileMode.Open);
                this.ErrorInfoList = (List<ErrorInfo>)Formatter.Deserialize(stream);
                stream.Close();

                this.RefreshAndSortLists();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Exception while loading database: " + ex.Message);
            }
        }

        private String Sanitize(String input)
        {
            input = input.Replace("\r", "\\r");
            input = input.Replace("\n", "\\n");
            input = input.Replace("\"", "'");
            return input;
        }

        private void Generate_Click(object sender, EventArgs e)
        {
            StringBuilder builder = new StringBuilder();

            int errorCount = 0;

            foreach (ErrorInfo info in this.ErrorInfoList)
            {
                builder.AppendFormat("{0} = {1},", info.Name, errorCount);
                builder.AppendLine();
                ++errorCount;
            }
            
            String codeHpp = builder.ToString();
            File.WriteAllText(@"..\..\..\..\Project\Zilch\ErrorDatabaseEnum.inl", codeHpp);


            builder = new StringBuilder();


            foreach (ErrorInfo info in this.ErrorInfoList)
            {
                builder.AppendFormat("// {0}", info.Name);
                builder.AppendLine();
                builder.AppendLine("{");
                builder.AppendLine("  ErrorInfo& error = this->Errors.push_back();");
                //builder.AppendFormat("  error.ErrorCode = ErrorCode::{0};", info.Name);
                //builder.AppendLine();
                builder.AppendFormat("  error.Error = \"{0}\";", Sanitize(info.Error));
                builder.AppendLine();
                builder.AppendFormat("  error.Name = \"{0}\";", Sanitize(info.Name));
                builder.AppendLine();
                builder.AppendFormat("  error.Reason = \"{0}\";", Sanitize(info.Reason));
                builder.AppendLine();
                builder.AppendLine();

                foreach (ErrorExample example in info.Examples)
                {
                    builder.AppendLine("  {");
                    builder.AppendLine("    ErrorExample& example = error.Examples.push_back();");
                    builder.AppendFormat("    example.ErrorCode = \"{0}\";", Sanitize(example.ErrorCode));
                    builder.AppendLine();
                    builder.AppendFormat("    example.FixedCode = \"{0}\";", Sanitize(example.FixedCode));
                    builder.AppendLine();
                    builder.AppendFormat("    example.ExplanationOfFix = \"{0}\";", Sanitize(example.ExplanationOfFix));
                    builder.AppendLine();
                    builder.AppendLine("  }");
                    builder.AppendLine();
                }

                //builder.AppendFormat("  error.Examples = {0},", info.Example);
                builder.AppendLine("}");
                builder.AppendLine();
            }

            String codeCpp = builder.ToString();
            File.WriteAllText(@"..\..\..\..\Project\Zilch\ErrorDatabaseSetup.inl", codeCpp);
        }

        private void ExampleList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (ExampleList.SelectedIndex != -1)
            {
                ErrorExample info = this.CurrentErrorInfo.Examples[ExampleList.SelectedIndex];

                this.CurrentExample = info;

                this.ExamplesGroup.Enabled = true;

                this.ErrorCode.Text = info.ErrorCode;
                this.FixedCode.Text = info.FixedCode;
                this.Brief.Text = info.ExplanationOfFix;
            }
            else
            {
                this.CurrentExample = null;

                this.ExamplesGroup.Enabled = false;

                this.ErrorCode.Text = "";
                this.FixedCode.Text = "";
                this.Brief.Text = "";
            }
        }

        private void AddExample_Click(object sender, EventArgs e)
        {
            ExampleList.Items.Add(ExampleList.Items.Count + 1);

            CurrentErrorInfo.Examples.Add(new ErrorExample());

            ExampleList.SelectedIndex = ExampleList.Items.Count - 1;
        }

        private void RemoveExample_Click(object sender, EventArgs e)
        {
            if (ExampleList.SelectedIndex != -1)
            {
                int temp = ExampleList.SelectedIndex;
                ExampleList.Items.RemoveAt(temp);

                CurrentErrorInfo.Examples.RemoveAt(temp);
                ExampleList.SelectedIndex = temp - 1;
            }
        }

        private void ErrorCode_TextChanged(object sender, EventArgs e)
        {
            if (this.CurrentExample != null)
            {
                this.CurrentExample.ErrorCode = ErrorCode.Text;
            }
        }

        private void FixedCode_TextChanged(object sender, EventArgs e)
        {
            if (this.CurrentExample != null)
            {
                this.CurrentExample.FixedCode = FixedCode.Text;
            }
        }

        private void Brief_TextChanged(object sender, EventArgs e)
        {
            if (this.CurrentExample != null && Brief.Text != null)
            {
                this.CurrentExample.ExplanationOfFix = Brief.Text;
            }
        }

        private void SearchText_TextChanged(object sender, EventArgs e)
        {
            var comparer = CultureInfo.CurrentCulture.CompareInfo;

            for (var i = 0; i < this.ErrorList.Items.Count; ++i)
            {
                var item = (String)this.ErrorList.Items[i];

                if (comparer.IndexOf(item, this.SearchText.Text, CompareOptions.IgnoreCase) != -1)
                {
                    this.ErrorList.SelectedIndex = i;
                    break;
                }
            }
        }
    }
}
