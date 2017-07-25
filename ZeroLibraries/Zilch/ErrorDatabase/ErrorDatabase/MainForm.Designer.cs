namespace ErrorDatabase
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.ErrorList = new System.Windows.Forms.ListBox();
            this.Add = new System.Windows.Forms.Button();
            this.Remove = new System.Windows.Forms.Button();
            this.Save = new System.Windows.Forms.Button();
            this.Generate = new System.Windows.Forms.Button();
            this.ErrorGroup = new System.Windows.Forms.GroupBox();
            this.ExamplesGroup = new System.Windows.Forms.GroupBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.Brief = new System.Windows.Forms.TextBox();
            this.FixedCode = new System.Windows.Forms.TextBox();
            this.ErrorCode = new System.Windows.Forms.TextBox();
            this.ExampleList = new System.Windows.Forms.ListBox();
            this.RemoveExample = new System.Windows.Forms.Button();
            this.AddExample = new System.Windows.Forms.Button();
            this.Reason = new System.Windows.Forms.TextBox();
            this.Error = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.NameText = new System.Windows.Forms.TextBox();
            this.SearchText = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.ErrorGroup.SuspendLayout();
            this.ExamplesGroup.SuspendLayout();
            this.SuspendLayout();
            // 
            // ErrorList
            // 
            this.ErrorList.FormattingEnabled = true;
            this.ErrorList.Location = new System.Drawing.Point(12, 48);
            this.ErrorList.Name = "ErrorList";
            this.ErrorList.Size = new System.Drawing.Size(233, 303);
            this.ErrorList.TabIndex = 12;
            this.ErrorList.SelectedIndexChanged += new System.EventHandler(this.ErrorList_SelectedIndexChanged);
            // 
            // Add
            // 
            this.Add.Location = new System.Drawing.Point(693, 361);
            this.Add.Name = "Add";
            this.Add.Size = new System.Drawing.Size(98, 24);
            this.Add.TabIndex = 11;
            this.Add.Text = "Add";
            this.Add.UseVisualStyleBackColor = true;
            this.Add.Click += new System.EventHandler(this.Add_Click);
            // 
            // Remove
            // 
            this.Remove.Location = new System.Drawing.Point(589, 361);
            this.Remove.Name = "Remove";
            this.Remove.Size = new System.Drawing.Size(98, 24);
            this.Remove.TabIndex = 10;
            this.Remove.Text = "Remove";
            this.Remove.UseVisualStyleBackColor = true;
            this.Remove.Click += new System.EventHandler(this.Remove_Click);
            // 
            // Save
            // 
            this.Save.Location = new System.Drawing.Point(485, 361);
            this.Save.Name = "Save";
            this.Save.Size = new System.Drawing.Size(98, 24);
            this.Save.TabIndex = 9;
            this.Save.Text = "Save";
            this.Save.UseVisualStyleBackColor = true;
            this.Save.Click += new System.EventHandler(this.Save_Click);
            // 
            // Generate
            // 
            this.Generate.Location = new System.Drawing.Point(381, 361);
            this.Generate.Name = "Generate";
            this.Generate.Size = new System.Drawing.Size(98, 24);
            this.Generate.TabIndex = 8;
            this.Generate.Text = "Generate";
            this.Generate.UseVisualStyleBackColor = true;
            this.Generate.Click += new System.EventHandler(this.Generate_Click);
            // 
            // ErrorGroup
            // 
            this.ErrorGroup.Controls.Add(this.ExamplesGroup);
            this.ErrorGroup.Controls.Add(this.ExampleList);
            this.ErrorGroup.Controls.Add(this.RemoveExample);
            this.ErrorGroup.Controls.Add(this.AddExample);
            this.ErrorGroup.Controls.Add(this.Reason);
            this.ErrorGroup.Controls.Add(this.Error);
            this.ErrorGroup.Controls.Add(this.label4);
            this.ErrorGroup.Controls.Add(this.label3);
            this.ErrorGroup.Controls.Add(this.label2);
            this.ErrorGroup.Controls.Add(this.label1);
            this.ErrorGroup.Controls.Add(this.NameText);
            this.ErrorGroup.Enabled = false;
            this.ErrorGroup.Location = new System.Drawing.Point(253, 6);
            this.ErrorGroup.Name = "ErrorGroup";
            this.ErrorGroup.Size = new System.Drawing.Size(538, 349);
            this.ErrorGroup.TabIndex = 23;
            this.ErrorGroup.TabStop = false;
            // 
            // ExamplesGroup
            // 
            this.ExamplesGroup.Controls.Add(this.label7);
            this.ExamplesGroup.Controls.Add(this.label6);
            this.ExamplesGroup.Controls.Add(this.label5);
            this.ExamplesGroup.Controls.Add(this.Brief);
            this.ExamplesGroup.Controls.Add(this.FixedCode);
            this.ExamplesGroup.Controls.Add(this.ErrorCode);
            this.ExamplesGroup.Enabled = false;
            this.ExamplesGroup.Location = new System.Drawing.Point(128, 140);
            this.ExamplesGroup.Name = "ExamplesGroup";
            this.ExamplesGroup.Size = new System.Drawing.Size(401, 192);
            this.ExamplesGroup.TabIndex = 24;
            this.ExamplesGroup.TabStop = false;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(6, 158);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(31, 13);
            this.label7.TabIndex = 42;
            this.label7.Text = "Brief:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(6, 87);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(35, 13);
            this.label6.TabIndex = 41;
            this.label6.Text = "Fixed:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(6, 16);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(32, 13);
            this.label5.TabIndex = 40;
            this.label5.Text = "Error:";
            // 
            // Brief
            // 
            this.Brief.Font = new System.Drawing.Font("Lucida Console", 9F);
            this.Brief.Location = new System.Drawing.Point(47, 158);
            this.Brief.Name = "Brief";
            this.Brief.Size = new System.Drawing.Size(345, 19);
            this.Brief.TabIndex = 4;
            this.Brief.TextChanged += new System.EventHandler(this.Brief_TextChanged);
            // 
            // FixedCode
            // 
            this.FixedCode.Font = new System.Drawing.Font("Lucida Console", 9F);
            this.FixedCode.Location = new System.Drawing.Point(47, 87);
            this.FixedCode.Multiline = true;
            this.FixedCode.Name = "FixedCode";
            this.FixedCode.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.FixedCode.Size = new System.Drawing.Size(345, 64);
            this.FixedCode.TabIndex = 3;
            this.FixedCode.TextChanged += new System.EventHandler(this.FixedCode_TextChanged);
            // 
            // ErrorCode
            // 
            this.ErrorCode.Font = new System.Drawing.Font("Lucida Console", 9F);
            this.ErrorCode.Location = new System.Drawing.Point(47, 15);
            this.ErrorCode.Multiline = true;
            this.ErrorCode.Name = "ErrorCode";
            this.ErrorCode.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.ErrorCode.Size = new System.Drawing.Size(345, 64);
            this.ErrorCode.TabIndex = 2;
            this.ErrorCode.TextChanged += new System.EventHandler(this.ErrorCode_TextChanged);
            // 
            // ExampleList
            // 
            this.ExampleList.FormattingEnabled = true;
            this.ExampleList.Location = new System.Drawing.Point(9, 146);
            this.ExampleList.Name = "ExampleList";
            this.ExampleList.Size = new System.Drawing.Size(113, 134);
            this.ExampleList.TabIndex = 5;
            this.ExampleList.SelectedIndexChanged += new System.EventHandler(this.ExampleList_SelectedIndexChanged);
            // 
            // RemoveExample
            // 
            this.RemoveExample.Location = new System.Drawing.Point(9, 311);
            this.RemoveExample.Name = "RemoveExample";
            this.RemoveExample.Size = new System.Drawing.Size(113, 21);
            this.RemoveExample.TabIndex = 7;
            this.RemoveExample.Text = "Remove Example";
            this.RemoveExample.UseVisualStyleBackColor = true;
            this.RemoveExample.Click += new System.EventHandler(this.RemoveExample_Click);
            // 
            // AddExample
            // 
            this.AddExample.Location = new System.Drawing.Point(9, 284);
            this.AddExample.Name = "AddExample";
            this.AddExample.Size = new System.Drawing.Size(113, 21);
            this.AddExample.TabIndex = 6;
            this.AddExample.Text = "Add Example";
            this.AddExample.UseVisualStyleBackColor = true;
            this.AddExample.Click += new System.EventHandler(this.AddExample_Click);
            // 
            // Reason
            // 
            this.Reason.Font = new System.Drawing.Font("Lucida Console", 9F);
            this.Reason.Location = new System.Drawing.Point(59, 68);
            this.Reason.Multiline = true;
            this.Reason.Name = "Reason";
            this.Reason.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.Reason.Size = new System.Drawing.Size(461, 52);
            this.Reason.TabIndex = 1;
            this.Reason.TextChanged += new System.EventHandler(this.Reason_TextChanged);
            // 
            // Error
            // 
            this.Error.Font = new System.Drawing.Font("Lucida Console", 9F);
            this.Error.Location = new System.Drawing.Point(59, 42);
            this.Error.Name = "Error";
            this.Error.Size = new System.Drawing.Size(461, 19);
            this.Error.TabIndex = 0;
            this.Error.TextChanged += new System.EventHandler(this.Error_TextChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 130);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(55, 13);
            this.label4.TabIndex = 27;
            this.label4.Text = "Examples:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 68);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(47, 13);
            this.label3.TabIndex = 26;
            this.label3.Text = "Reason:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 42);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(32, 13);
            this.label2.TabIndex = 25;
            this.label2.Text = "Error:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(38, 13);
            this.label1.TabIndex = 24;
            this.label1.Text = "Name:";
            // 
            // NameText
            // 
            this.NameText.Font = new System.Drawing.Font("Lucida Console", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.NameText.Location = new System.Drawing.Point(59, 16);
            this.NameText.Name = "NameText";
            this.NameText.Size = new System.Drawing.Size(461, 19);
            this.NameText.TabIndex = 13;
            this.NameText.TextChanged += new System.EventHandler(this.NameText_TextChanged);
            // 
            // SearchText
            // 
            this.SearchText.Location = new System.Drawing.Point(56, 16);
            this.SearchText.Name = "SearchText";
            this.SearchText.Size = new System.Drawing.Size(188, 20);
            this.SearchText.TabIndex = 24;
            this.SearchText.TextChanged += new System.EventHandler(this.SearchText_TextChanged);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(8, 16);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(44, 13);
            this.label8.TabIndex = 25;
            this.label8.Text = "Search:";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 393);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.SearchText);
            this.Controls.Add(this.ErrorGroup);
            this.Controls.Add(this.Generate);
            this.Controls.Add(this.Save);
            this.Controls.Add(this.Remove);
            this.Controls.Add(this.Add);
            this.Controls.Add(this.ErrorList);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Error Code";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ErrorGroup.ResumeLayout(false);
            this.ErrorGroup.PerformLayout();
            this.ExamplesGroup.ResumeLayout(false);
            this.ExamplesGroup.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox ErrorList;
        private System.Windows.Forms.Button Add;
        private System.Windows.Forms.Button Remove;
        private System.Windows.Forms.Button Save;
        private System.Windows.Forms.Button Generate;
        private System.Windows.Forms.GroupBox ErrorGroup;
        private System.Windows.Forms.GroupBox ExamplesGroup;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox Brief;
        private System.Windows.Forms.TextBox FixedCode;
        private System.Windows.Forms.TextBox ErrorCode;
        private System.Windows.Forms.ListBox ExampleList;
        private System.Windows.Forms.Button RemoveExample;
        private System.Windows.Forms.Button AddExample;
        private System.Windows.Forms.TextBox Reason;
        private System.Windows.Forms.TextBox Error;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox NameText;
        private System.Windows.Forms.TextBox SearchText;
        private System.Windows.Forms.Label label8;
    }
}

