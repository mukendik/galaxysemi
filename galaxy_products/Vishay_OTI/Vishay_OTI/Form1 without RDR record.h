#pragma once
#define DISABLE_PROGRAM 1  // Set to 1 to disable program after expiration date
						   // Set to 0 to enable program to run after expiration date
#include "profile.h"

namespace Vishay_OTI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Diagnostics;
	using namespace System::IO;
	using namespace System::Runtime::InteropServices;
	using namespace System::Globalization;
	
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		
		Form1(void)
		{
			InitializeComponent();	
			// 
			// dataGridView1
			// 
			array<String^>^ headers1 = {L"Lot_ID", L"Date_Code", L"Package", L"Equipment_ID", 
				L"Product_ID", L"Site_ID", L"Geom_Name", L"Pkg_Type", L"Lot_ID_2",L"Geom_Name_2", 
				L"Lot_ID_3",L"Geom_Name_3", L"Lot_ID_4",L"Geom_Name_4"};  
			dataGridView1->ColumnCount = headers1 -> Length;
			for (int i=0; i < headers1->Length; i++) 
				dataGridView1->Columns[i]->Name=headers1[i];
			dataGridView1->Columns["Lot_ID"]->Width = 80;
			dataGridView1->Columns["Date_Code"]->Width = 75;
			dataGridView1->Columns["Package"]->Width = 95;
			dataGridView1->Columns["Equipment_ID"]->Width = 90;
			dataGridView1->Columns["Product_ID"]->Width = 140;
			dataGridView1->Columns["Site_ID"]->Width = 55;
			dataGridView1->Columns["Geom_Name"]->Width = 120;
			dataGridView1->Dock = DockStyle::Fill;
			dataGridView1->AlternatingRowsDefaultCellStyle->BackColor = Color::AliceBlue;
			dataGridView1->AlternatingRowsDefaultCellStyle->ForeColor = Color::Black;
			// dataGridView1->AutoResizeColumns();
			// 
			// dataGridView2
			// 
			dataGridView2->AlternatingRowsDefaultCellStyle->BackColor = Color::AliceBlue;
			dataGridView2->AlternatingRowsDefaultCellStyle->ForeColor = Color::Black;
			dataGridView2->AutoResizeColumns();
			// 
			// dataGridView3
			// 
			dataGridView3->AlternatingRowsDefaultCellStyle->BackColor = Color::AliceBlue;
			dataGridView3->AlternatingRowsDefaultCellStyle->ForeColor = Color::Black;
			dataGridView3->AutoResizeColumns();
		}
	
	protected:
		
		/// Clean up any resources being used.

		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	//
	// Global variables
	//
	static String^ strFT_PromisDataFile = "";
	static String^ CSVdataPathEV = "OTI_PATH";
	static String^ TriggerPathEV = "OTI_OUTPUT_ATDF";
	static String^ CSVPromisFile = "Promis_Data_FT.txt";
	static String^ CSVFinalTestsFile = "CSVFinalTestsFile.csv";
	static String^ CSVSortEntriesFile = "CSVSortEntriesFile.csv";
	static String^ CSVEquipID_Tester = "CSV_Tester_ID.csv";
	static String^ CSVEquipID_IF89 = "CSV_IF8-IF9_ID.csv";
	static String^ CSVEquipID_UIS = "CSV_UIS_ID.csv";
	static String^ CSVEquipID_LCR = "CSV_LCR_ID.csv";
	static String^ CSVEquipID_TR = "CSV_TR_ID.csv";
	static String^ CSVEquipID_UIS_2 = "CSV_UIS_ID_2.csv";
	static String^ CSVEquipID_LCR_2 = "CSV_LCR_ID_2.csv";
	static String^ CSVEquipID_TR_2 = "CSV_TR_ID_2.csv";
	static String^ DotINIfileName = "INI_OTI.ini";
	static String^ GALdataType = "FT";
	static String^ reTestBinList = "1-65535";
	static String^ GALfileType = ".atd";
	static int TimerInterval = 3 * 60 * 1000;	// PROMIS file polling interval (Milliseconds)
	//
	// Fields that must be present in Final Test and Sort Entries data files
	//
	static array<String^>^ neededFields = {"TEST NAME ", "TEST NUMBER", "MAPPED TEST", "BIN NUMBER"};
	static array<Char>^ chrComment = {';','0'};  // CSV comment character		
	//
	// Windows Form Designer generated objects
	//
	private: System::Windows::Forms::DataGridView^  dataGridView1;
	private: System::Windows::Forms::TabControl^  tabControl1;
	private: System::Windows::Forms::TabPage^  tabPage1;
	private: System::Windows::Forms::TabPage^  tabPage2;
	private: System::Windows::Forms::TabPage^  tabPage3;
	private: System::Windows::Forms::Button^  btnValidate;
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::CheckBox^  checkBox1;
	private: System::Windows::Forms::Label^  label1;

	private: System::Windows::Forms::Label^  Test_Retest;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::Label^  failCount;
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::DataGridView^  dataGridView2;
	private: System::Windows::Forms::Label^  FT_failCount;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::TextBox^  totalParts;
	private: System::Windows::Forms::DataGridView^  dataGridView3;
	private: System::Windows::Forms::Label^  SE_failCount;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  siteID;
	private: System::Windows::Forms::Panel^  panel2;

	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Test_Family;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Test_Name;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Test_Number;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Mapped_Test;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Bin_Number;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Fail_Count;
	private: System::Windows::Forms::Button^  btnCancelFT;
	private: System::Windows::Forms::Button^  btnCancelSE;
	private: System::Windows::Forms::Timer^  timer1;
	private: System::ComponentModel::IContainer^  components;	
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  SE_Test_Family;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  SE_Test_Name;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  SE_Test_Number;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  SE_Mapped_Test;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  SE_Bin_Number;

	private: System::Windows::Forms::ComboBox^  TesterComboBox;
	private: System::Windows::Forms::ComboBox^  IF89_ComboBox;
private: System::Windows::Forms::ComboBox^  UIS_ComboBox;


	private: System::Windows::Forms::ComboBox^  TR_ComboBox;
	private: System::Windows::Forms::ComboBox^  LCR_ComboBox;

	private: System::Windows::Forms::Label^  label9;
	private: System::Windows::Forms::Label^  label10;
	private: System::Windows::Forms::Label^  label11;
	private: System::Windows::Forms::Label^  label12;
	private: System::Windows::Forms::Label^  label13;
private: System::Windows::Forms::GroupBox^  groupBox1;
private: System::Windows::Forms::Label^  label16;
private: System::Windows::Forms::Label^  label8;
private: System::Windows::Forms::Label^  label14;
private: System::Windows::Forms::ComboBox^  LCR_2ComboBox;
private: System::Windows::Forms::ComboBox^  UIS_2ComboBox;
private: System::Windows::Forms::Label^  label15;
private: System::Windows::Forms::ComboBox^  TR_2ComboBox;
private: System::Windows::Forms::Panel^  MD_EquipIDs;
private: System::Windows::Forms::DataGridViewTextBoxColumn^  SE_Fail_Count;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle1 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle2 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle3 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle4 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			this->dataGridView1 = (gcnew System::Windows::Forms::DataGridView());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage1 = (gcnew System::Windows::Forms::TabPage());
			this->btnCancelFT = (gcnew System::Windows::Forms::Button());
			this->FT_failCount = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->dataGridView2 = (gcnew System::Windows::Forms::DataGridView());
			this->Test_Family = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Test_Name = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Test_Number = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Mapped_Test = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Bin_Number = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Fail_Count = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->tabPage2 = (gcnew System::Windows::Forms::TabPage());
			this->btnCancelSE = (gcnew System::Windows::Forms::Button());
			this->dataGridView3 = (gcnew System::Windows::Forms::DataGridView());
			this->SE_Test_Family = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->SE_Test_Name = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->SE_Test_Number = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->SE_Mapped_Test = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->SE_Bin_Number = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->SE_Fail_Count = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->SE_failCount = (gcnew System::Windows::Forms::Label());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->tabPage3 = (gcnew System::Windows::Forms::TabPage());
			this->btnValidate = (gcnew System::Windows::Forms::Button());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->Test_Retest = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->failCount = (gcnew System::Windows::Forms::Label());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->totalParts = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->siteID = (gcnew System::Windows::Forms::Label());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->TesterComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->IF89_ComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->UIS_ComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->TR_ComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->LCR_ComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->label12 = (gcnew System::Windows::Forms::Label());
			this->label13 = (gcnew System::Windows::Forms::Label());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->MD_EquipIDs = (gcnew System::Windows::Forms::Panel());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->label16 = (gcnew System::Windows::Forms::Label());
			this->TR_2ComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->label15 = (gcnew System::Windows::Forms::Label());
			this->label14 = (gcnew System::Windows::Forms::Label());
			this->UIS_2ComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->LCR_2ComboBox = (gcnew System::Windows::Forms::ComboBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView1))->BeginInit();
			this->tabControl1->SuspendLayout();
			this->tabPage1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView2))->BeginInit();
			this->tabPage2->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView3))->BeginInit();
			this->tabPage3->SuspendLayout();
			this->panel1->SuspendLayout();
			this->panel2->SuspendLayout();
			this->groupBox1->SuspendLayout();
			this->MD_EquipIDs->SuspendLayout();
			this->SuspendLayout();
			// 
			// dataGridView1
			// 
			this->dataGridView1->AllowUserToAddRows = false;
			this->dataGridView1->AllowUserToDeleteRows = false;
			this->dataGridView1->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridView1->Location = System::Drawing::Point(43, 25);
			this->dataGridView1->Name = L"dataGridView1";
			this->dataGridView1->ReadOnly = true;
			this->dataGridView1->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->dataGridView1->Size = System::Drawing::Size(280, 150);
			this->dataGridView1->TabIndex = 5;
			this->dataGridView1->RowHeaderMouseDoubleClick += gcnew System::Windows::Forms::DataGridViewCellMouseEventHandler(this, &Form1::dataGridView1_RowHeaderMouseDoubleClick);
			this->dataGridView1->CellMouseDoubleClick += gcnew System::Windows::Forms::DataGridViewCellMouseEventHandler(this, &Form1::dataGridView1_CellMouseDoubleClick);
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPage1);
			this->tabControl1->Controls->Add(this->tabPage2);
			this->tabControl1->Controls->Add(this->tabPage3);
			this->tabControl1->Location = System::Drawing::Point(14, 118);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(797, 435);
			this->tabControl1->TabIndex = 11;
			// 
			// tabPage1
			// 
			this->tabPage1->Controls->Add(this->btnCancelFT);
			this->tabPage1->Controls->Add(this->FT_failCount);
			this->tabPage1->Controls->Add(this->label6);
			this->tabPage1->Controls->Add(this->dataGridView2);
			this->tabPage1->Location = System::Drawing::Point(4, 22);
			this->tabPage1->Name = L"tabPage1";
			this->tabPage1->Padding = System::Windows::Forms::Padding(3);
			this->tabPage1->Size = System::Drawing::Size(789, 409);
			this->tabPage1->TabIndex = 0;
			this->tabPage1->Text = L"Final Tests";
			this->tabPage1->UseVisualStyleBackColor = true;
			// 
			// btnCancelFT
			// 
			this->btnCancelFT->Location = System::Drawing::Point(660, 219);
			this->btnCancelFT->Name = L"btnCancelFT";
			this->btnCancelFT->Size = System::Drawing::Size(75, 23);
			this->btnCancelFT->TabIndex = 13;
			this->btnCancelFT->Text = L"Cancel FT";
			this->btnCancelFT->UseVisualStyleBackColor = true;
			this->btnCancelFT->Click += gcnew System::EventHandler(this, &Form1::btnCancelFT_Click);
			// 
			// FT_failCount
			// 
			this->FT_failCount->AutoSize = true;
			this->FT_failCount->BackColor = System::Drawing::SystemColors::Info;
			this->FT_failCount->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->FT_failCount->Location = System::Drawing::Point(644, 191);
			this->FT_failCount->MinimumSize = System::Drawing::Size(100, 15);
			this->FT_failCount->Name = L"FT_failCount";
			this->FT_failCount->Size = System::Drawing::Size(100, 15);
			this->FT_failCount->TabIndex = 0;
			this->FT_failCount->Text = L"0";
			this->FT_failCount->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(651, 169);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(83, 13);
			this->label6->TabIndex = 0;
			this->label6->Text = L"FT Fail Count";
			// 
			// dataGridView2
			// 
			this->dataGridView2->AllowUserToAddRows = false;
			this->dataGridView2->AllowUserToDeleteRows = false;
			dataGridViewCellStyle1->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
			dataGridViewCellStyle1->BackColor = System::Drawing::SystemColors::Control;
			dataGridViewCellStyle1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			dataGridViewCellStyle1->ForeColor = System::Drawing::SystemColors::WindowText;
			dataGridViewCellStyle1->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle1->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle1->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->dataGridView2->ColumnHeadersDefaultCellStyle = dataGridViewCellStyle1;
			this->dataGridView2->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridView2->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(6) {this->Test_Family, 
				this->Test_Name, this->Test_Number, this->Mapped_Test, this->Bin_Number, this->Fail_Count});
			this->dataGridView2->Location = System::Drawing::Point(158, 19);
			this->dataGridView2->Name = L"dataGridView2";
			this->dataGridView2->Size = System::Drawing::Size(453, 374);
			this->dataGridView2->TabIndex = 12;
			this->dataGridView2->CellValidated += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &Form1::dataGridView2_CellValidated);
			this->dataGridView2->CellValidating += gcnew System::Windows::Forms::DataGridViewCellValidatingEventHandler(this, &Form1::dataGridView2_CellValidating);
			this->dataGridView2->EditingControlShowing += gcnew System::Windows::Forms::DataGridViewEditingControlShowingEventHandler(this, &Form1::dataGridView2_EditingControlShowing);
			// 
			// Test_Family
			// 
			this->Test_Family->HeaderText = L"Test_Family";
			this->Test_Family->Name = L"Test_Family";
			this->Test_Family->ReadOnly = true;
			// 
			// Test_Name
			// 
			this->Test_Name->HeaderText = L"Test_Name";
			this->Test_Name->Name = L"Test_Name";
			this->Test_Name->ReadOnly = true;
			// 
			// Test_Number
			// 
			this->Test_Number->HeaderText = L"Test_Number";
			this->Test_Number->Name = L"Test_Number";
			this->Test_Number->ReadOnly = true;
			// 
			// Mapped_Test
			// 
			this->Mapped_Test->HeaderText = L"Mapped_Test";
			this->Mapped_Test->Name = L"Mapped_Test";
			this->Mapped_Test->ReadOnly = true;
			this->Mapped_Test->Visible = false;
			// 
			// Bin_Number
			// 
			this->Bin_Number->HeaderText = L"Bin_Number";
			this->Bin_Number->Name = L"Bin_Number";
			this->Bin_Number->ReadOnly = true;
			this->Bin_Number->Visible = false;
			// 
			// Fail_Count
			// 
			dataGridViewCellStyle2->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleRight;
			this->Fail_Count->DefaultCellStyle = dataGridViewCellStyle2;
			this->Fail_Count->HeaderText = L"Fail_Count";
			this->Fail_Count->Name = L"Fail_Count";
			// 
			// tabPage2
			// 
			this->tabPage2->Controls->Add(this->btnCancelSE);
			this->tabPage2->Controls->Add(this->dataGridView3);
			this->tabPage2->Controls->Add(this->SE_failCount);
			this->tabPage2->Controls->Add(this->label7);
			this->tabPage2->Location = System::Drawing::Point(4, 22);
			this->tabPage2->Name = L"tabPage2";
			this->tabPage2->Padding = System::Windows::Forms::Padding(3);
			this->tabPage2->Size = System::Drawing::Size(789, 409);
			this->tabPage2->TabIndex = 1;
			this->tabPage2->Text = L"Sort Entries";
			this->tabPage2->UseVisualStyleBackColor = true;
			// 
			// btnCancelSE
			// 
			this->btnCancelSE->Location = System::Drawing::Point(660, 219);
			this->btnCancelSE->Name = L"btnCancelSE";
			this->btnCancelSE->Size = System::Drawing::Size(75, 23);
			this->btnCancelSE->TabIndex = 13;
			this->btnCancelSE->Text = L"Cancel SE";
			this->btnCancelSE->UseVisualStyleBackColor = true;
			this->btnCancelSE->Click += gcnew System::EventHandler(this, &Form1::btnCancelSE_Click);
			// 
			// dataGridView3
			// 
			this->dataGridView3->AllowUserToAddRows = false;
			this->dataGridView3->AllowUserToDeleteRows = false;
			dataGridViewCellStyle3->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
			dataGridViewCellStyle3->BackColor = System::Drawing::SystemColors::Control;
			dataGridViewCellStyle3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			dataGridViewCellStyle3->ForeColor = System::Drawing::SystemColors::WindowText;
			dataGridViewCellStyle3->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle3->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle3->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->dataGridView3->ColumnHeadersDefaultCellStyle = dataGridViewCellStyle3;
			this->dataGridView3->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridView3->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(6) {this->SE_Test_Family, 
				this->SE_Test_Name, this->SE_Test_Number, this->SE_Mapped_Test, this->SE_Bin_Number, this->SE_Fail_Count});
			this->dataGridView3->Location = System::Drawing::Point(158, 19);
			this->dataGridView3->Name = L"dataGridView3";
			this->dataGridView3->Size = System::Drawing::Size(453, 374);
			this->dataGridView3->TabIndex = 12;
			this->dataGridView3->CellValidated += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &Form1::dataGridView3_CellValidated);
			this->dataGridView3->CellValidating += gcnew System::Windows::Forms::DataGridViewCellValidatingEventHandler(this, &Form1::dataGridView3_CellValidating);
			this->dataGridView3->EditingControlShowing += gcnew System::Windows::Forms::DataGridViewEditingControlShowingEventHandler(this, &Form1::dataGridView3_EditingControlShowing);
			// 
			// SE_Test_Family
			// 
			this->SE_Test_Family->HeaderText = L"Test_Family";
			this->SE_Test_Family->Name = L"SE_Test_Family";
			this->SE_Test_Family->ReadOnly = true;
			// 
			// SE_Test_Name
			// 
			this->SE_Test_Name->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::ColumnHeader;
			this->SE_Test_Name->HeaderText = L"Test_Name";
			this->SE_Test_Name->Name = L"SE_Test_Name";
			this->SE_Test_Name->ReadOnly = true;
			this->SE_Test_Name->Width = 96;
			// 
			// SE_Test_Number
			// 
			this->SE_Test_Number->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::ColumnHeader;
			this->SE_Test_Number->HeaderText = L"Test_Number";
			this->SE_Test_Number->Name = L"SE_Test_Number";
			this->SE_Test_Number->ReadOnly = true;
			this->SE_Test_Number->Width = 107;
			// 
			// SE_Mapped_Test
			// 
			this->SE_Mapped_Test->HeaderText = L"Mapped_Test";
			this->SE_Mapped_Test->Name = L"SE_Mapped_Test";
			this->SE_Mapped_Test->ReadOnly = true;
			this->SE_Mapped_Test->Visible = false;
			// 
			// SE_Bin_Number
			// 
			this->SE_Bin_Number->HeaderText = L"Bin_Number";
			this->SE_Bin_Number->Name = L"SE_Bin_Number";
			this->SE_Bin_Number->ReadOnly = true;
			this->SE_Bin_Number->Visible = false;
			// 
			// SE_Fail_Count
			// 
			this->SE_Fail_Count->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::ColumnHeader;
			dataGridViewCellStyle4->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleRight;
			this->SE_Fail_Count->DefaultCellStyle = dataGridViewCellStyle4;
			this->SE_Fail_Count->HeaderText = L"Fail_Count";
			this->SE_Fail_Count->Name = L"SE_Fail_Count";
			this->SE_Fail_Count->Width = 92;
			// 
			// SE_failCount
			// 
			this->SE_failCount->AutoSize = true;
			this->SE_failCount->BackColor = System::Drawing::SystemColors::Info;
			this->SE_failCount->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->SE_failCount->Location = System::Drawing::Point(644, 191);
			this->SE_failCount->MinimumSize = System::Drawing::Size(100, 15);
			this->SE_failCount->Name = L"SE_failCount";
			this->SE_failCount->Size = System::Drawing::Size(100, 15);
			this->SE_failCount->TabIndex = 0;
			this->SE_failCount->Text = L"0";
			this->SE_failCount->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(651, 169);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(84, 13);
			this->label7->TabIndex = 0;
			this->label7->Text = L"SE Fail Count";
			// 
			// tabPage3
			// 
			this->tabPage3->Controls->Add(this->dataGridView1);
			this->tabPage3->Location = System::Drawing::Point(4, 22);
			this->tabPage3->Name = L"tabPage3";
			this->tabPage3->Padding = System::Windows::Forms::Padding(3);
			this->tabPage3->Size = System::Drawing::Size(789, 409);
			this->tabPage3->TabIndex = 2;
			this->tabPage3->Text = L"FT_Promis Data File";
			this->tabPage3->UseVisualStyleBackColor = true;
			// 
			// btnValidate
			// 
			this->btnValidate->Location = System::Drawing::Point(316, 570);
			this->btnValidate->Name = L"btnValidate";
			this->btnValidate->Size = System::Drawing::Size(87, 23);
			this->btnValidate->TabIndex = 14;
			this->btnValidate->Text = L"Validate";
			this->btnValidate->UseVisualStyleBackColor = true;
			this->btnValidate->Click += gcnew System::EventHandler(this, &Form1::btnValidate_Click);
			// 
			// btnCancel
			// 
			this->btnCancel->Location = System::Drawing::Point(427, 570);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(87, 23);
			this->btnCancel->TabIndex = 15;
			this->btnCancel->Text = L"Cancel All";
			this->btnCancel->UseVisualStyleBackColor = true;
			this->btnCancel->Click += gcnew System::EventHandler(this, &Form1::btnCancel_Click);
			// 
			// comboBox1
			// 
			this->comboBox1->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Location = System::Drawing::Point(16, 23);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(152, 21);
			this->comboBox1->TabIndex = 0;
			this->comboBox1->SelectionChangeCommitted += gcnew System::EventHandler(this, &Form1::comboBox1_SelectionChangeCommitted);
			this->comboBox1->Leave += gcnew System::EventHandler(this, &Form1::comboBox1_Leave);
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->Location = System::Drawing::Point(16, 58);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(75, 17);
			this->checkBox1->TabIndex = 2;
			this->checkBox1->Text = L"Full Test";
			this->checkBox1->UseVisualStyleBackColor = true;
			this->checkBox1->Visible = false;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(14, 4);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(104, 13);
			this->label1->TabIndex = 8;
			this->label1->Text = L"Last Test Completed\r\n";
			// 
			// Test_Retest
			// 
			this->Test_Retest->AutoSize = true;
			this->Test_Retest->BackColor = System::Drawing::SystemColors::Info;
			this->Test_Retest->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->Test_Retest->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->Test_Retest->Location = System::Drawing::Point(17, 22);
			this->Test_Retest->MinimumSize = System::Drawing::Size(100, 15);
			this->Test_Retest->Name = L"Test_Retest";
			this->Test_Retest->Size = System::Drawing::Size(100, 15);
			this->Test_Retest->TabIndex = 9;
			this->Test_Retest->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->Location = System::Drawing::Point(45, 5);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(95, 16);
			this->label3->TabIndex = 10;
			this->label3->Text = L"Lot_ID.Split#";
			this->label3->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label4->Location = System::Drawing::Point(175, 4);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(137, 16);
			this->label4->TabIndex = 11;
			this->label4->Text = L"Total Parts Tested";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->BackColor = System::Drawing::SystemColors::ActiveBorder;
			this->label5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label5->Location = System::Drawing::Point(364, 605);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(97, 13);
			this->label5->TabIndex = 13;
			this->label5->Text = L"Total Fail Count";
			// 
			// failCount
			// 
			this->failCount->AutoSize = true;
			this->failCount->BackColor = System::Drawing::SystemColors::Info;
			this->failCount->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->failCount->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->failCount->Location = System::Drawing::Point(364, 620);
			this->failCount->MinimumSize = System::Drawing::Size(100, 15);
			this->failCount->Name = L"failCount";
			this->failCount->Size = System::Drawing::Size(100, 15);
			this->failCount->TabIndex = 14;
			this->failCount->Text = L"0";
			this->failCount->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// panel1
			// 
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel1->Controls->Add(this->label1);
			this->panel1->Controls->Add(this->Test_Retest);
			this->panel1->Location = System::Drawing::Point(671, 594);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(136, 43);
			this->panel1->TabIndex = 15;
			this->panel1->Visible = false;
			// 
			// totalParts
			// 
			this->totalParts->Location = System::Drawing::Point(178, 23);
			this->totalParts->Name = L"totalParts";
			this->totalParts->Size = System::Drawing::Size(130, 20);
			this->totalParts->TabIndex = 1;
			this->totalParts->Text = L"0";
			this->totalParts->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->totalParts->TextChanged += gcnew System::EventHandler(this, &Form1::totalParts_TextChanged);
			this->totalParts->Validated += gcnew System::EventHandler(this, &Form1::totalParts_Validated);
			this->totalParts->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Form1::totalParts_KeyPress);
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(38, 4);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(39, 13);
			this->label2->TabIndex = 16;
			this->label2->Text = L"Site ID";
			// 
			// siteID
			// 
			this->siteID->AutoSize = true;
			this->siteID->BackColor = System::Drawing::SystemColors::Info;
			this->siteID->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->siteID->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->siteID->Location = System::Drawing::Point(11, 22);
			this->siteID->MinimumSize = System::Drawing::Size(100, 15);
			this->siteID->Name = L"siteID";
			this->siteID->Size = System::Drawing::Size(100, 15);
			this->siteID->TabIndex = 17;
			this->siteID->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// panel2
			// 
			this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel2->Controls->Add(this->label2);
			this->panel2->Controls->Add(this->siteID);
			this->panel2->Location = System::Drawing::Point(18, 594);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(122, 43);
			this->panel2->TabIndex = 18;
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer_Tick);
			// 
			// TesterComboBox
			// 
			this->TesterComboBox->FormattingEnabled = true;
			this->TesterComboBox->Location = System::Drawing::Point(21, 39);
			this->TesterComboBox->Name = L"TesterComboBox";
			this->TesterComboBox->Size = System::Drawing::Size(90, 21);
			this->TesterComboBox->TabIndex = 3;
			// 
			// IF89_ComboBox
			// 
			this->IF89_ComboBox->FormattingEnabled = true;
			this->IF89_ComboBox->Location = System::Drawing::Point(111, 39);
			this->IF89_ComboBox->Name = L"IF89_ComboBox";
			this->IF89_ComboBox->Size = System::Drawing::Size(90, 21);
			this->IF89_ComboBox->TabIndex = 4;
			// 
			// UIS_ComboBox
			// 
			this->UIS_ComboBox->FormattingEnabled = true;
			this->UIS_ComboBox->Location = System::Drawing::Point(292, 39);
			this->UIS_ComboBox->Name = L"UIS_ComboBox";
			this->UIS_ComboBox->Size = System::Drawing::Size(90, 21);
			this->UIS_ComboBox->TabIndex = 6;
			// 
			// TR_ComboBox
			// 
			this->TR_ComboBox->FormattingEnabled = true;
			this->TR_ComboBox->Location = System::Drawing::Point(381, 39);
			this->TR_ComboBox->Name = L"TR_ComboBox";
			this->TR_ComboBox->Size = System::Drawing::Size(90, 21);
			this->TR_ComboBox->TabIndex = 7;
			// 
			// LCR_ComboBox
			// 
			this->LCR_ComboBox->FormattingEnabled = true;
			this->LCR_ComboBox->Location = System::Drawing::Point(202, 39);
			this->LCR_ComboBox->Name = L"LCR_ComboBox";
			this->LCR_ComboBox->Size = System::Drawing::Size(90, 21);
			this->LCR_ComboBox->TabIndex = 5;
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->Location = System::Drawing::Point(42, 20);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(51, 13);
			this->label9->TabIndex = 25;
			this->label9->Text = L"Tester";
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Location = System::Drawing::Point(130, 21);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(47, 13);
			this->label10->TabIndex = 26;
			this->label10->Text = L"IF8-IF9";
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->Location = System::Drawing::Point(321, 20);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(28, 13);
			this->label11->TabIndex = 27;
			this->label11->Text = L"UIS";
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->Location = System::Drawing::Point(413, 21);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(24, 13);
			this->label12->TabIndex = 29;
			this->label12->Text = L"TR";
			// 
			// label13
			// 
			this->label13->AutoSize = true;
			this->label13->Location = System::Drawing::Point(227, 21);
			this->label13->Name = L"label13";
			this->label13->Size = System::Drawing::Size(31, 13);
			this->label13->TabIndex = 28;
			this->label13->Text = L"LCR";
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->MD_EquipIDs);
			this->groupBox1->Controls->Add(this->TesterComboBox);
			this->groupBox1->Controls->Add(this->label12);
			this->groupBox1->Controls->Add(this->IF89_ComboBox);
			this->groupBox1->Controls->Add(this->label13);
			this->groupBox1->Controls->Add(this->UIS_ComboBox);
			this->groupBox1->Controls->Add(this->LCR_ComboBox);
			this->groupBox1->Controls->Add(this->label11);
			this->groupBox1->Controls->Add(this->TR_ComboBox);
			this->groupBox1->Controls->Add(this->label9);
			this->groupBox1->Controls->Add(this->label10);
			this->groupBox1->Location = System::Drawing::Point(331, 5);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(480, 117);
			this->groupBox1->TabIndex = 3;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Equipment IDs";
			// 
			// MD_EquipIDs
			// 
			this->MD_EquipIDs->Controls->Add(this->label8);
			this->MD_EquipIDs->Controls->Add(this->label16);
			this->MD_EquipIDs->Controls->Add(this->TR_2ComboBox);
			this->MD_EquipIDs->Controls->Add(this->label15);
			this->MD_EquipIDs->Controls->Add(this->label14);
			this->MD_EquipIDs->Controls->Add(this->UIS_2ComboBox);
			this->MD_EquipIDs->Controls->Add(this->LCR_2ComboBox);
			this->MD_EquipIDs->Location = System::Drawing::Point(53, 66);
			this->MD_EquipIDs->Name = L"MD_EquipIDs";
			this->MD_EquipIDs->Size = System::Drawing::Size(421, 45);
			this->MD_EquipIDs->TabIndex = 36;
			this->MD_EquipIDs->Visible = false;
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Location = System::Drawing::Point(354, 0);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(38, 13);
			this->label8->TabIndex = 39;
			this->label8->Text = L"TR_2";
			// 
			// label16
			// 
			this->label16->AutoSize = true;
			this->label16->Location = System::Drawing::Point(11, 21);
			this->label16->Name = L"label16";
			this->label16->Size = System::Drawing::Size(118, 13);
			this->label16->TabIndex = 35;
			this->label16->Text = L"Multi-die Equipment";
			// 
			// TR_2ComboBox
			// 
			this->TR_2ComboBox->FormattingEnabled = true;
			this->TR_2ComboBox->Location = System::Drawing::Point(328, 18);
			this->TR_2ComboBox->Name = L"TR_2ComboBox";
			this->TR_2ComboBox->Size = System::Drawing::Size(90, 21);
			this->TR_2ComboBox->TabIndex = 10;
			// 
			// label15
			// 
			this->label15->AutoSize = true;
			this->label15->Location = System::Drawing::Point(173, 0);
			this->label15->Name = L"label15";
			this->label15->Size = System::Drawing::Size(45, 13);
			this->label15->TabIndex = 37;
			this->label15->Text = L"LCR_2";
			// 
			// label14
			// 
			this->label14->AutoSize = true;
			this->label14->Location = System::Drawing::Point(260, 0);
			this->label14->Name = L"label14";
			this->label14->Size = System::Drawing::Size(42, 13);
			this->label14->TabIndex = 38;
			this->label14->Text = L"UIS_2";
			// 
			// UIS_2ComboBox
			// 
			this->UIS_2ComboBox->FormattingEnabled = true;
			this->UIS_2ComboBox->Location = System::Drawing::Point(238, 18);
			this->UIS_2ComboBox->Name = L"UIS_2ComboBox";
			this->UIS_2ComboBox->Size = System::Drawing::Size(90, 21);
			this->UIS_2ComboBox->TabIndex = 9;
			// 
			// LCR_2ComboBox
			// 
			this->LCR_2ComboBox->FormattingEnabled = true;
			this->LCR_2ComboBox->Location = System::Drawing::Point(148, 18);
			this->LCR_2ComboBox->Name = L"LCR_2ComboBox";
			this->LCR_2ComboBox->Size = System::Drawing::Size(90, 21);
			this->LCR_2ComboBox->TabIndex = 8;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(7, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(827, 648);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->panel2);
			this->Controls->Add(this->totalParts);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->failCount);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->checkBox1);
			this->Controls->Add(this->comboBox1);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->btnValidate);
			this->Controls->Add(this->tabControl1);
			this->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->Name = L"Form1";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Vishay OTI";
			this->Shown += gcnew System::EventHandler(this, &Form1::Form1_Shown);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView1))->EndInit();
			this->tabControl1->ResumeLayout(false);
			this->tabPage1->ResumeLayout(false);
			this->tabPage1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView2))->EndInit();
			this->tabPage2->ResumeLayout(false);
			this->tabPage2->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dataGridView3))->EndInit();
			this->tabPage3->ResumeLayout(false);
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel2->PerformLayout();
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->MD_EquipIDs->ResumeLayout(false);
			this->MD_EquipIDs->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

private: System::Void Form1_Shown(System::Object^  sender, System::EventArgs^  e) 
{
	String^ dataPath; 

	dataPath = Environment::GetEnvironmentVariable( CSVdataPathEV );
	System::Windows::Forms::DialogResult result;
	if (dataPath == nullptr) // Has the path to the TXT/CSV files been set?
	{
		result = MessageBox::Show("The Environment Variable (OTI_PATH) that identifies"+ "\n" +
			"the path to the PROMIS and Test files has not been set." + "\n" + "\n"
			+ "Operator must select the correct directory." + "\n" + "\n" + 
			"Note: PROMIS and Test files must reside in the same directory." + "\n" + "\n" +
			"Select OK to continue or Cancel to end this session.",
			"Environment Variable Not Set", MessageBoxButtons::OKCancel, 
			MessageBoxIcon::Exclamation );
		if ( result == System::Windows::Forms::DialogResult::Cancel )
		{		
			this->Close();  // Close the parent form
			return;
		}
		// Select a directory
		else			
		{		
			dataPath = BrowseToDirectory();
			if ( dataPath == nullptr )				
				this->Close();  // Closes the parent form
		}
	}
	else
	{
		// Environment Variable is set, does the path exist?
		if (!Directory::Exists(dataPath))
		{	
			System::Windows::Forms::DialogResult result = 
				MessageBox::Show("Path indicated by the Environment Variable (OTI_PATH)"+ "\n" + 
				"that identifies the location of the PROMIS and Test files does not exist." + "\n" + "\n" + 
				"Operator must select the correct directory." + "\n" + "\n" + 
				"Note: PROMIS and Test files must reside in the same directory." + "\n" + "\n" +
				"Select OK to continue or Cancel to end this session.",
				"Environment Variable Problem", MessageBoxButtons::OKCancel, 
				MessageBoxIcon::Exclamation );
			if ( result == System::Windows::Forms::DialogResult::Cancel )
			{		
				this->Close();  // Close the parent form
				return;
			}	
			else			// Select a directory
			{	
				dataPath = BrowseToDirectory();
				if ( dataPath == nullptr )			
					this->Close();  // Closes the parent form
			}
		}
	}
	if (!dataPath->EndsWith("\\"))
			dataPath += "\\";

	strFT_PromisDataFile = dataPath + CSVPromisFile;
	if (!File::Exists(strFT_PromisDataFile))
	{		
		do 
		{
			System::Windows::Forms::DialogResult result = 
				MessageBox::Show("PROMIS data file was not found." + "\n" + "\n" + 
				"Operator must locate the PROMIS data file in order to continue." +
				"\n" + "\n" + "Note: PROMIS and Test files must reside in the same directory." + 
				"\n" + "\n" + "Select OK to continue or Cancel to end this session.",
				"PROMIS TXT File Location Problem", MessageBoxButtons::OKCancel, 
				MessageBoxIcon::Exclamation );
			if ( result == System::Windows::Forms::DialogResult::Cancel )
			{		
				this->Close();  // Close the parent form
				return;
			}
			dataPath = BrowseToFile( CSVPromisFile );
		} while (dataPath == nullptr);
		strFT_PromisDataFile = dataPath + CSVPromisFile;
	}
	comboBox1->BeginUpdate();
	if (!getPromisCSVdata())
	{
		this->Close();
		return;
	}
	comboBox1->EndUpdate();

	//
	// Load Final Test data
	//
	String^ strTestDataFile = dataPath + CSVFinalTestsFile;
	if (!loadTestFile("Final Test", strTestDataFile, dataGridView2))
	{
		this->Close();
		return;
	}
	//
	// Load Sort Entries data
	//
	strTestDataFile = dataPath + CSVSortEntriesFile;
	if (!loadTestFile("Sort Entries", strTestDataFile, dataGridView3))
	{
		this->Close();
		return;
	}
	//
	// Load Equipment Combo Boxes
	//	
	if (!loadEquipmentIDs(dataPath + CSVEquipID_Tester, TesterComboBox))
	{
		this->Close();
		return;
	}
	if (!loadEquipmentIDs(dataPath + CSVEquipID_IF89, IF89_ComboBox))
	{
		this->Close();
		return;
	}
	if (!loadEquipmentIDs(dataPath + CSVEquipID_UIS, UIS_ComboBox))
	{
		this->Close();
		return;
	}
	if (!loadEquipmentIDs(dataPath + CSVEquipID_LCR, LCR_ComboBox))
	{
		this->Close();
		return;
	}
	if (!loadEquipmentIDs(dataPath + CSVEquipID_TR, TR_ComboBox))
	{
		this->Close();
		return;
	}
	if (!loadEquipmentIDs(dataPath + CSVEquipID_UIS_2, UIS_2ComboBox))
	{
		this->Close();
		return;
	}
	if (!loadEquipmentIDs(dataPath + CSVEquipID_LCR_2, LCR_2ComboBox))
	{
		this->Close();
		return;
	}
	if (!loadEquipmentIDs(dataPath + CSVEquipID_TR_2, TR_2ComboBox))
	{
		this->Close();
		return;
	}
	//
	// Start the clock used to refresh the PROMIS file DataGridView
	//
	this->timer1->Interval = TimerInterval; // In milliseconds
	this->timer1->Start();
	//
	// Initialize and synchronize comboBox, Retest Info and PROMIS dataGridView
	//
	this->comboBox1->Select();
	this->comboBox1->SelectedIndex = 0;
	getRetest(comboBox1->SelectedItem);
	this->dataGridView1->FirstDisplayedScrollingRowIndex = 0;
	this->siteID->Text = dataGridView1->Rows[0]->Cells["Site_ID"]->Value->ToString();
	this->MD_EquipIDs->Visible = isMultiDie(dataGridView1->Rows[0]->
		Cells["Pkg_Type"]->Value->ToString());
}

private: System::Void comboBox1_SelectionChangeCommitted
			 (System::Object^  sender, System::EventArgs^  e) 
{
	int selectedIndex = comboBox1->SelectedIndex;
	Object^ selectedItem = comboBox1->SelectedItem;
	this->checkBox1->Checked = false;
	getRetest(comboBox1->SelectedItem);
	this->siteID->Text = dataGridView1->Rows[selectedIndex]->Cells["Site_ID"]->
		Value->ToString();
	this->MD_EquipIDs->Visible = isMultiDie(dataGridView1->Rows[selectedIndex]->
		Cells["Pkg_Type"]->Value->ToString());
	this->dataGridView1->FirstDisplayedScrollingRowIndex = selectedIndex;
	dataGridView1->CurrentCell->Selected = false; 
	dataGridView1->Rows[selectedIndex]->Cells[0]->Selected = true; 
	dataGridView1->CurrentCell = dataGridView1->SelectedCells[0];
}

private: void getRetest(System::Object^ objLotID_Split)
{

	int intReTest;
	String^ strINI = getStrINIpath() + "\\" + DotINIfileName; 
	String^ strLotID = objLotID_Split->ToString();
	
	// Marshal the managed string to unmanaged memory
	char* chrLotID = (char*)(void*)Marshal::StringToHGlobalAnsi(strLotID);
	char* chrINI = (char*)(void*)Marshal::StringToHGlobalAnsi(strINI);
	
	// Use unmanaged string to obtain .INI information from unmanaged C code
	intReTest = get_private_profile_int(chrLotID, "Retest", -1, chrINI);
	this->panel1->Visible = true;
	if (intReTest == -1)
	{	
		this->Test_Retest->Text = "None";
		this->checkBox1->Visible = false;
	}
	else if (intReTest == 0)
		{	
			this->Test_Retest->Text = "Full Test";
			this->checkBox1->Visible = true;
		}
		else
		{
			this->Test_Retest->Text = "Retest: " + intReTest.ToString();
			this->checkBox1->Visible = true;
		}

	Marshal::FreeHGlobal((IntPtr)(chrLotID)); // Free the unmanaged string
	Marshal::FreeHGlobal((IntPtr)(chrINI));
}
//
// totalParts Events
//

private: System::Void totalParts_TextChanged(System::Object^  sender, System::EventArgs^  e) 
{
	String^ strValue;
	int newInteger;

	strValue = this->totalParts->Text->Trim();
	if (!String::IsNullOrEmpty(strValue))
	{
		if (!Int32::TryParse(strValue, newInteger) || newInteger < 0)
		{	
			MessageBox::Show("Total parts tested count must be a positive number.", 
				"Data Entry Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);	
			this->totalParts->Text = "0";
		}
	}	 
}

private: System::Void totalParts_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) 
{
	if (!(Char::IsDigit(e->KeyChar)))
	{
		Keys key = (Keys)e->KeyChar;
		if( key != Keys::Back ) 
        {
			e->Handled = true;
        }
    }
}

private: System::Void totalParts_Validated(System::Object^  sender, System::EventArgs^  e) 
{
	String^ strValue = this->totalParts->Text->Trim();
	if (String::IsNullOrEmpty(strValue))	
		this->totalParts->Text = "0";
}
//
//	dataGridView2 Final Test Events
//
private: System::Void dataGridView2_EditingControlShowing(System::Object^  sender, 
			 System::Windows::Forms::DataGridViewEditingControlShowingEventArgs^  e) 
{
	int ind = this->dataGridView2->CurrentCell->ColumnIndex;
	if (dataGridView2->Columns[ind]->Name == "Fail_Count")
	{
		TextBox^ tb = dynamic_cast<TextBox^>(e->Control);
		tb->KeyPress -= gcnew KeyPressEventHandler(this, &Vishay_OTI::Form1::tb_KeyPress);
    	tb->KeyPress += gcnew KeyPressEventHandler(this, &Vishay_OTI::Form1::tb_KeyPress);
	}
}

private: System::Void dataGridView2_CellValidating(System::Object^  sender, System::Windows::Forms::DataGridViewCellValidatingEventArgs^  e) 
{
	String^ strValue;
	int newInteger;
	int intOldCount;
	int intNewCount;
	int intRevisedFTCount;

    if (dataGridView2->Columns[e->ColumnIndex]->Name == "Fail_Count")
	{
		strValue = e->FormattedValue->ToString();
		if (!String::IsNullOrEmpty(strValue))
		{	
			if (!Int32::TryParse(strValue, newInteger) || newInteger < 0)
			{	
				MessageBox::Show("Fail count must be a positive number.", 
					"Data Entry Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);	
				e->Cancel = true;
			}
			else
			{
				// Last value stored
				intOldCount = System::Convert::ToInt32
					(dataGridView2->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value );
				// Value to be stored
				intNewCount = System::Convert::ToInt32 (strValue);			
				// Revised total is the last total - last value for this row + new value
				intRevisedFTCount = System::Convert::ToInt32 
					(this->FT_failCount->Text) - intOldCount + intNewCount;
				this->FT_failCount->Text = intRevisedFTCount.ToString();
			}
		}
		else
			e->Cancel = true;
	}
}    
	
private: System::Void dataGridView2_CellValidated(System::Object^  sender, 
			 System::Windows::Forms::DataGridViewCellEventArgs^  e) 
{	
	String^ strValue;
	int intValue;
	int intTotalFailCount;

	if (dataGridView2->Columns[e->ColumnIndex]->Name == "Fail_Count")
	{
		
		strValue = dataGridView2->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value->
			ToString()->Trim();
		intValue = System::Convert::ToInt32 (strValue);
		// Remove leading zeros
		if ( strValue->StartsWith( "0" ) )			
			dataGridView2->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value = 
				intValue.ToString();
		
		intTotalFailCount = System::Convert::ToInt32(this->FT_failCount->Text)
			+ System::Convert::ToInt32(this->SE_failCount->Text);
		this->failCount->Text = intTotalFailCount.ToString();		
	}
}
//
//	dataGridView3 Sort Entries Events
//
private: System::Void dataGridView3_EditingControlShowing(System::Object^  sender, 
			 System::Windows::Forms::DataGridViewEditingControlShowingEventArgs^  e)
{
	int ind = this->dataGridView3->CurrentCell->ColumnIndex;
	if (dataGridView3->Columns[ind]->Name == "SE_Fail_Count")
	{
		TextBox^ tb = dynamic_cast<TextBox^>(e->Control);
		tb->KeyPress -= gcnew KeyPressEventHandler(this, &Vishay_OTI::Form1::tb_KeyPress);
    	tb->KeyPress += gcnew KeyPressEventHandler(this, &Vishay_OTI::Form1::tb_KeyPress);
	}
}

private: System::Void dataGridView3_CellValidating(System::Object^  sender, System::Windows::Forms::DataGridViewCellValidatingEventArgs^  e) 
{
	String^ strValue;
	int newInteger;
	int intOldCount;
	int intNewCount;
	int intRevisedDSCount;

    if (dataGridView3->Columns[e->ColumnIndex]->Name == "SE_Fail_Count")
	{
		strValue = e->FormattedValue->ToString();
		if (!String::IsNullOrEmpty(strValue))
		{	
			if (!Int32::TryParse(strValue, newInteger) || newInteger < 0)
			{	
				MessageBox::Show("Fail count must be a positive number.", 
					"Data Entry Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);	
				e->Cancel = true;
			}
			else
			{
				// Last value stored
				intOldCount = System::Convert::ToInt32
					(dataGridView3->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value->ToString());
				// Value to be stored
				intNewCount = System::Convert::ToInt32 (strValue);			
				// Revised total is the last total - last value for this row + new value
				intRevisedDSCount = System::Convert::ToInt32 
					(this->SE_failCount->Text) - intOldCount + intNewCount;
				this->SE_failCount->Text = intRevisedDSCount.ToString();
			}
		}
		else
			e->Cancel = true;
	}
}    
	
private: System::Void dataGridView3_CellValidated(System::Object^  sender, 
			 System::Windows::Forms::DataGridViewCellEventArgs^  e) 
{	
	String^ strValue;
	int intValue;
	int intTotalFailCount;

	if (dataGridView3->Columns[e->ColumnIndex]->Name == "SE_Fail_Count")
	{
		
		strValue = dataGridView3->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value->
			ToString()->Trim();
		intValue = System::Convert::ToInt32 (strValue);
		// Remove leading zeros
		if ( strValue->StartsWith( "0" ) )			
			dataGridView3->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value = 
				intValue.ToString();
		
		intTotalFailCount = System::Convert::ToInt32(this->FT_failCount->Text)
			+ System::Convert::ToInt32(this->SE_failCount->Text);
		this->failCount->Text = intTotalFailCount.ToString();		
	}
}

//
//	Utility Routines
//

private: void tb_KeyPress(System::Object ^, KeyPressEventArgs^ e)
{
	if (!(Char::IsDigit(e->KeyChar)))
	{
		Keys key = (Keys)e->KeyChar;
		if( key != Keys::Back ) 
        {
			e->Handled = true;
        }
    }
}

private: void zeroAll()
{
	// Clear Fail Counts
	int intRows2 = this->dataGridView2->Rows->Count;
	int intCell2  = this->dataGridView2->ColumnCount - 1;
	for (int i = 0; i < intRows2; i++)
		this->dataGridView2->Rows[i]->Cells[intCell2]->Value = "0";
	// Clear Sort Entries
	intRows2 = this->dataGridView3->Rows->Count;
	intCell2  = this->dataGridView3->ColumnCount - 1;
	for (int i = 0; i < intRows2; i++)
		this->dataGridView3->Rows[i]->Cells[intCell2]->Value = "0";
	
	this->failCount->Text = L"0";
	this->FT_failCount->Text = L"0";
	this->SE_failCount->Text = L"0";
	this->totalParts->Text = L"0";
	TesterComboBox->SelectedIndex = 0;
	IF89_ComboBox->SelectedIndex = 0;
	UIS_ComboBox->SelectedIndex = 0;
	LCR_ComboBox->SelectedIndex = 0;
	TR_ComboBox->SelectedIndex = 0;
	UIS_2ComboBox->SelectedIndex = 0;
	LCR_2ComboBox->SelectedIndex = 0;
	TR_2ComboBox->SelectedIndex = 0;
	this->tabControl1->SelectedTab = tabPage1;
	comboBox1->BeginUpdate();
	dataGridView1->Rows->Clear();
	comboBox1->Items->Clear();
	if (!getPromisCSVdata())
	{
		this->Close();
		return;
	}
	comboBox1->EndUpdate();
	this->comboBox1->Select();
	this->comboBox1->SelectedIndex = 0;
	this->checkBox1->Visible = false;
	this->checkBox1->Checked = false;
	getRetest(comboBox1->SelectedItem);
	
	dataGridView1->FirstDisplayedScrollingRowIndex = 0;
	dataGridView1->CurrentCell->Selected = false; 
	dataGridView1->Rows[0]->Cells[0]->Selected = true; 
	dataGridView1->CurrentCell = dataGridView1->SelectedCells[0]; 
	this->siteID->Text = dataGridView1->Rows[0]->Cells["Site_ID"]->Value->ToString();
	this->MD_EquipIDs->Visible = isMultiDie(dataGridView1->Rows[0]->
		Cells["Pkg_Type"]->Value->ToString());
	dataGridView2->FirstDisplayedScrollingRowIndex = 0;
	dataGridView2->CurrentCell->Selected = false; 
	dataGridView2->Rows[0]->Cells[0]->Selected = true; 
	dataGridView2->CurrentCell = dataGridView2->SelectedCells[0]; 

	dataGridView3->FirstDisplayedScrollingRowIndex = 0;
	dataGridView3->CurrentCell->Selected = false; 
	dataGridView3->Rows[0]->Cells[0]->Selected = true; 
	dataGridView3->CurrentCell = dataGridView3->SelectedCells[0]; 
}

private: String^ BrowseToDirectory()
{	
	String^ newPath;
	
	FolderBrowserDialog^ folderBrowserDialog1 = gcnew FolderBrowserDialog;  
	// Set the help text description for the FolderBrowserDialog
	folderBrowserDialog1->Description = "Select the default directory";
      
	// Do not allow the user to create new files via the FolderBrowserDialog.
	folderBrowserDialog1->ShowNewFolderButton = false;
      
	// Default to the My Computer folder.
	folderBrowserDialog1->RootFolder = Environment::SpecialFolder::MyComputer;
	
	// Show the FolderBrowserDialog.
	System::Windows::Forms::DialogResult result = folderBrowserDialog1->ShowDialog();
	if ( result == System::Windows::Forms::DialogResult::OK )
		newPath = folderBrowserDialog1->SelectedPath + "\\";
	else
		newPath = nullptr;
	return newPath;
}

private: String^ BrowseToFile(String^ fileToGet)
{
	String^ newPath;
	String^ strFileName;
	
	String^ RootDirectory = Environment::GetEnvironmentVariable("HOMEDRIVE") + "\\";
	OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;
	openFileDialog1->DefaultExt = "txt";
    openFileDialog1->Filter = "TXT files (*.txt)|*.txt";
	openFileDialog1->InitialDirectory = RootDirectory;
    openFileDialog1->FilterIndex = 1;
	openFileDialog1->CheckPathExists = true;
    openFileDialog1->RestoreDirectory = true;
	
	if ( openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
    {
		strFileName = Path::GetFileName(openFileDialog1->FileName)->ToUpper();
		if (strFileName != fileToGet->ToUpper())
			newPath = nullptr;
		else
			newPath = Path::GetDirectoryName(openFileDialog1->FileName)+ "\\";
	}
	else
		newPath = nullptr;
	return newPath;
}

private: String^ getStrINIpath()
{
	String ^ strINIpath = Environment::GetEnvironmentVariable("ALLUSERSPROFILE");
	return strINIpath;
}

private: System::Void btnCancelSE_Click(System::Object^  
			 sender, System::EventArgs^  e) 
{
	
	int intRows3 = this->dataGridView3->Rows->Count;
	int intCell3  = this->dataGridView3->ColumnCount - 1;
	for (int i = 0; i < intRows3; i++)
		this->dataGridView3->Rows[i]->Cells[intCell3]->Value = "0";
	this->SE_failCount->Text = L"0";
	this->failCount->Text = this->FT_failCount->Text;
	this->dataGridView3->FirstDisplayedScrollingRowIndex = 0;
}

private: System::Void btnCancelFT_Click(System::Object^  sender, 
			 System::EventArgs^  e) 
{
	int intRows2 = this->dataGridView2->Rows->Count;
	int intCell2 = this->dataGridView2->ColumnCount - 1;
	for (int i = 0; i < intRows2; i++)
		this->dataGridView2->Rows[i]->Cells[intCell2]->Value = "0";
	this->FT_failCount->Text = L"0";
	this->failCount->Text = this->SE_failCount->Text;
	this->dataGridView2->FirstDisplayedScrollingRowIndex = 0;
}

private: void timer_Tick(Object^ sender, System::EventArgs^ e)
{
	array<String^>^ s = gcnew array<String^>(14);
	int row = this->comboBox1->SelectedIndex;
	if (row > 0)
	{
		int cellCount = dataGridView1->Rows[row]->Cells->Count;
		for (int index = 0; index < cellCount; index++)
			s[index] = dataGridView1->Rows[row]->Cells[index]->Value->ToString();
 
		comboBox1->BeginUpdate();
		comboBox1->Items->Clear();
		dataGridView1->Rows->Clear();
		dataGridView1->Rows->Add(s);
		comboBox1->Items->Add( s[0] );
		if (!getPromisCSVdata())
		{
			this->Close();
			return;
		}
		comboBox1->EndUpdate();	
		this->comboBox1->SelectedIndex = 0;
		getRetest(comboBox1->SelectedItem);
		this->dataGridView1->FirstDisplayedScrollingRowIndex = 0;
		this->siteID->Text = dataGridView1->Rows[0]->Cells["Site_ID"]->Value->ToString();
	}
}

private: bool getPromisCSVdata()
{
	StreamReader^ din;
	String^ str;
	
	array<String^>^ items = gcnew array<String^>(14);
	int num_items = items->Length;
	array<Char>^chars =  {','};
	
	timer1->Stop();
	try 
	{
		din = File::OpenText(strFT_PromisDataFile);
	}
	catch (Exception^ e)
	{
		if (dynamic_cast<FileNotFoundException^>(e))
			MessageBox::Show("PROMIS Data file was not found. Program will close.", 
			"File Not Found", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);				
		else
			MessageBox::Show("PROMIS Data file cannot be opened. Program will close.", 
			"File Corrupted", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return false;
	}
	while ((str = din->ReadLine()) != nullptr) 
	{	
		if (str->IndexOfAny(chrComment) != 0)
		{
			array<String^>^s = str->Split(chars);
			for (int i = 0; i < num_items; i++)
			{
				if (i < s->Length)
					items[i] = s[i]->Trim();
				else
					items[i] = "";
			}
			dataGridView1->Rows->Add(items);			
			comboBox1->Items->Add( items[0] );
		}
	}
	timer1->Start();
	return true;
}

private: System::Void comboBox1_Leave
			 (System::Object^  sender, System::EventArgs^  e)
{
	comboBox1_SelectionChangeCommitted(sender, e);
}

//
// Button Events
//
private: System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e) 
{
	zeroAll();		 
}

private: System::Void btnValidate_Click(System::Object^  sender, 
			 System::EventArgs^  e) 
{

	String^ strTriggerFile;
	String^ strTriggerPath;
	String^ strINIpath;
	
	//
	// Program Evaluation Time Limit Logic
	//	
#if (DISABLE_PROGRAM == 1)
	DateTime currentTime = DateTime::Now;
	// Expiration Date is 6 Apr 2009 (12 Noon)
	DateTime dateToExpire = DateTime(2009, 5, 29, 12, 0, 0); 
	if ( DateTime::Compare( currentTime, dateToExpire ) >  0 )
	{	
		MessageBox::Show("OTI program evaluation period has expired. Program will close.", 
			"Please Contact Galaxy", MessageBoxButtons::OK, MessageBoxIcon::Stop);	
		this->Close();  // Close the parent form
		return;
	}
#endif
	
	strTriggerPath = nullptr;
	strTriggerPath = Environment::GetEnvironmentVariable( TriggerPathEV );
	strINIpath = getStrINIpath();
	// If the path has not been set use the INI file path
	if (strTriggerPath == nullptr)
		strTriggerPath = strINIpath;
	strTriggerPath += "\\";
	String^ strLot_ID = this->comboBox1->Text;
	
	int goodParts = finalErrorCheck();
	if (goodParts >= 0)
	{		
		strTriggerFile = buildFileName(strLot_ID);
		writeTriggerFile(strTriggerPath + strTriggerFile, goodParts);
		updateINIfile(strINIpath, strLot_ID);	
		zeroAll();
	}
}

private: int finalErrorCheck()
{
	int intTotalFailCount = System::Convert::ToInt32(this->failCount->Text);
	if (String::IsNullOrEmpty(this->totalParts->Text))
		this->totalParts->Text = "0";
	int intTotalParts = System::Convert::ToInt32(this->totalParts->Text);
	int goodParts = intTotalParts - intTotalFailCount;
	//
	// Check that there are more parts than failures
	//
	if (goodParts < 0)
	{
		MessageBox::Show(" Total parts tested must be greater than or equal to the number of failures.\n\n" + 
			"Total Fail Count: " + this->failCount->Text + "\n\n" +	
			"Total Parts Tested: " + this->totalParts->Text + "\n\n" + 
			"Trigger File was not generated.",
			"Data Entry Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return -1;
	}
	//
	// Check that total parts and failure data have been entered
	//
	if (intTotalParts == 0 && intTotalFailCount == 0)
	{
		MessageBox::Show(" No entries were made for Total Parts Tested or Failures.\n\n" + 
			"Total Fail Count: " + this->failCount->Text + "\n\n" +	
			"Total Parts Tested: " + this->totalParts->Text + "\n\n" + 
			"Trigger File was not generated.",
			"Data Entry Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return -1;
	}
	//
	//	Check that at least one equipment type has been entered
	//
	if (TesterComboBox->Text == "" && IF89_ComboBox->Text == "" &&
		UIS_ComboBox->Text == "" && LCR_ComboBox->Text == "" && 
		TR_ComboBox->Text == "" && UIS_2ComboBox->Text == "" && 
		LCR_2ComboBox->Text == "" && TR_2ComboBox->Text == "")
	{
		MessageBox::Show(" No equipment IDs were entered.\n\n" + 
			"At least one item must be selected.\n\n" + 
			"Trigger File was not generated.",
			"Data Entry Error", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return -1;
	}
	return goodParts;
}

private: String^ buildFileName(String^ strLot_ID)
{
	String^ fileNameTrg;
	
	// Set Retest instance
	String^ nowTestInstance = this->Test_Retest->Text;
	String^ reTestInstance = setRetestInstance(nowTestInstance);
		
	// Construct trigger file name
	fileNameTrg = strLot_ID + "_" + GALdataType + "_" + reTestInstance+ "_" + 
		reTestBinList + GALfileType;

	return fileNameTrg; 
}

private: void writeTriggerFile(String^ strTriggerFile, int goodParts)
{
	String^ str;
	String^ strLot_ID = dataGridView1->CurrentRow->Cells["Lot_ID"]->Value->ToString();
	String^ strDate_Code = dataGridView1->CurrentRow->Cells["Date_Code"]->Value->ToString();
	String^ strPackage = dataGridView1->CurrentRow->Cells["Package"]->Value->ToString();
	String^ strEquipment_ID  = dataGridView1->CurrentRow->Cells["Equipment_ID"]->Value->ToString();
	String^ strProduct_ID  = dataGridView1->CurrentRow->Cells["Product_ID"]->Value->ToString();
	String^ strSite_ID = dataGridView1->CurrentRow->Cells["Site_ID"]->Value->ToString();;
	String^ strGeom_Name  = dataGridView1->CurrentRow->Cells["Geom_Name"]->Value->ToString();
	String^ strPkg_Type = dataGridView1->CurrentRow->Cells["Pkg_Type"]->Value->ToString();
	String^ strLot_ID_2 = dataGridView1->CurrentRow->Cells["Lot_ID_2"]->Value->ToString();
	String^ strGeom_Name_2  = dataGridView1->CurrentRow->Cells["Geom_Name_2"]->Value->ToString();
	String^ strLot_ID_3 = dataGridView1->CurrentRow->Cells["Lot_ID_3"]->Value->ToString();
	String^ strGeom_Name_3  = dataGridView1->CurrentRow->Cells["Geom_Name_3"]->Value->ToString();
	String^ strLot_ID_4 = dataGridView1->CurrentRow->Cells["Lot_ID_4"]->Value->ToString();
	String^ strGeom_Name_4  = dataGridView1->CurrentRow->Cells["Geom_Name_4"]->Value->ToString();
	array<String^>^ Upper_Months = {"JAN", "FEB", "MAR", "APR", "MAY",
		"JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

	StreamWriter ^sw = gcnew StreamWriter(gcnew FileStream(strTriggerFile, FileMode::Create,
		FileAccess::ReadWrite, FileShare::None));

	DateTime currentTime = DateTime::Now;
	String^ strCurrentTime = currentTime.Now.ToString("HH:mm:ss");
	int year = currentTime.Year;
	int month = currentTime.Month;
	int day = currentTime.Day;
	// Construct field avoiding local representation of the month
	String^ strCT = strCurrentTime + " " + day.ToString() + "-" + Upper_Months[month - 1] + 
		"-" + year.ToString();

	String^ Exec_Cnt = this->totalParts->Text;

	// FAR Record (3 fields)
	// e.g., FAR:A|4|2|
	String^ FARrecord ("FAR:A|4|2|");
	sw->WriteLine(FARrecord);
	
	// MIR Records (39 fields)
	// e.g., MIR:pre_lu|RF5000|ESD|hp84k1|Hp84000|15:19:31 21-JAN-2003|
		// 15:19:35 21-JAN-2003|galaxy|D|0||||Dec 16 2002 13:14:14|hp84000_server|
		// A.04.03||||||/home/galaxy/testplan.sl||||||||Icc.lim|Dec 16 2002 11:46:19|
		// ||||||
	
	String^ reTestCode = getRetestCode();
	String^ mod_COD = "P";
	String^ stat_NUM = "1";
	array<Char>^ delim =  {'.'};
	array<String^>^ s = strLot_ID->Split(delim);
	String^ strLot_ID_NoSplit = s[0];
	str = "MIR:" + strLot_ID_NoSplit + "|" + strGeom_Name + "|||" + strEquipment_ID + "|" 
		+ strCT + "|" + strCT + "||" + mod_COD + "|" + stat_NUM + "|" + strLot_ID + "||" 
		+ reTestCode + "||||||||||" + strPkg_Type + "||" + strDate_Code + "|" + strSite_ID 
		+ "||" + strProduct_ID + "||||||||||";
	sw->WriteLine(str);
	
	// SDR Record (19 Fields)
	// e.g., SDR:2|4|5,6,7,8|Delta Flex|D511||B101|17
	String^ str_HAND_ID = TesterComboBox->Text;
	String^ str_CARD_ID = IF89_ComboBox->Text;
	String^ str_LOAD_ID = UIS_ComboBox->Text;
	String^ str_DIB_ID = LCR_ComboBox->Text;
	String^ str_CABL_ID = TR_ComboBox->Text;
	String^ str_CONT_ID = UIS_2ComboBox->Text;
	String^ str_LASR_ID = LCR_2ComboBox->Text;
	String^ str_EXTR_ID = TR_2ComboBox->Text;

	str = "SDR:||||" + str_HAND_ID + "||" + str_CARD_ID + "||" + str_LOAD_ID +
		"||" + str_DIB_ID + "||" + str_CABL_ID + "||" + str_CONT_ID + "||" +
		str_LASR_ID + "||" + str_EXTR_ID;
	sw->WriteLine(str);

	// TSR Records (16 fields)
	// e.g., TSR:|1|0|GT1:Icc:DC Current, A:|P|22|||||||||	
	Hashtable ^hashBins = gcnew Hashtable(200);  // Save failures in hash table
	// Final Tests
	buildTSRrecord(dataGridView2, Exec_Cnt, sw, hashBins);
	// Sort Entries
	buildTSRrecord(dataGridView3, Exec_Cnt, sw, hashBins);
	
	//
	// SBR Records (7 fields)
	// e.g., SBR:||0|5|F|Gain
	str = "SBR:||1|" + goodParts.ToString() + "|P|";
	sw->WriteLine(str);
	
	IDictionaryEnumerator ^enum1 = hashBins->GetEnumerator();		
	while ( enum1->MoveNext() )
	{
		String^ Bin = enum1->Key->ToString();
		String^ Count = enum1->Value->ToString();
		if (Count != "0")
		{
			str = "SBR:||" + Bin + "|" + Count + "|F|";
			sw->WriteLine(str);
		}
	}
	//
	//	Multi-die DTR Records (1 field per die)
	//	e.g., DTR:<cmd> multi-die die=1;wafer_product=PARF20FFOA;wafer_lot=X16M139.9
	//	1S = single die, 2D = 2 dies, 2P = 2 dies, 3T = 3 dies, 4P = 4 dies
	//
		if (strPkg_Type != "1S")			// No DTR record generated for single die
		{
			//
			// Multi-die wafer has at least 2 dies
			//
			str = "DTR:<cmd> multi-die die=1;wafer_product=" + strGeom_Name + 
				";wafer_lot=" + strLot_ID;
			sw->WriteLine(str);								// Write DTR for die 1
			str = "DTR:<cmd> multi-die die=2;wafer_product=" + strGeom_Name_2 + 
				";wafer_lot=" + strLot_ID_2;
			sw->WriteLine(str);								// Write DTR for die 2

			if (strPkg_Type == "3T" || strPkg_Type == "4P")	// Does this wafer have more than 2 dies?
			{
				str = "DTR:<cmd> multi-die die=3;wafer_product=" + strGeom_Name_3 + 
					";wafer_lot=" + strLot_ID_3;
				sw->WriteLine(str);							// Write DTR for die 3
				if (strPkg_Type == "4P")					// Does this wafer have 4 dies
				{
					str = "DTR:<cmd> multi-die die=4;wafer_product=" + strGeom_Name_4 + 
						";wafer_lot=" + strLot_ID_4;
					sw->WriteLine(str);						// Write DTR for die 4
				}
			}
		}
	//
	// MRR Records (5 fields)
	// e.g., MRR:15:33:20 21-JAN-2003|||
	str = "MRR:" + strCT + "|||";
	sw->WriteLine(str);
	sw->Close();	
}

private: void buildTSRrecord(DataGridView^ dataGridView, String^ Exec_Cnt, 
			 StreamWriter^ sw, Hashtable ^hashBins)
{
	String^ str;
	String^ strGrid;
	
	if (dataGridView->Name == "dataGridView3")
		strGrid = "SE_";
	else
		strGrid ="";

	for (int i = 0; i < dataGridView->Rows->Count; i++)
	{
		String^ Test_Num = dataGridView->Rows[i]->Cells[strGrid + "Mapped_Test"]->
			Value->ToString();
		String^ Test_Name = dataGridView->Rows[i]->Cells[strGrid + "Test_Name"]->
			Value->ToString();
		String^ Fail_Count = dataGridView->Rows[i]->Cells[strGrid + "Fail_Count"]->
			Value->ToString();
		if (Fail_Count != "0")
		{
			str = "TSR:||" + Test_Num + "|" + Test_Name + "|P|" + Exec_Cnt + "|" + 
				Fail_Count + "|||||||||";
			sw->WriteLine(str);
			//
			// Calculate Bin totals
			//
			String^ Mapped_Test = dataGridView->Rows[i]->Cells[strGrid + "Mapped_Test"]->
				Value->ToString();
			String^ Bin_Number = dataGridView->Rows[i]->Cells[strGrid + "Bin_Number"]->
				Value->ToString();
			AddToBin(hashBins, Bin_Number, Fail_Count);
		}
	}
	return;
}

private: String^ getRetestCode()
{
	if (this->Test_Retest->Text == "None")
		return "N";
	if (this->Test_Retest->Text == "Full Test")
		return "Y";
	int ind = this->Test_Retest->Text->IndexOf(":");
	String^ strValue = this->Test_Retest->Text->Substring(ind + 1)->Trim();
	int intValue = System::Convert::ToInt32(strValue);
	if (intValue > 9)
		intValue = 9;
	strValue = intValue.ToString();
	return strValue;
}

private: void updateINIfile(String^ strINIpath, String^ strLot_ID)
{ 
	String^ strINI = strINIpath + "\\" + DotINIfileName;
	
	// Marshal the managed string to unmanaged memory
	char* chrLotID = (char*)(void*)Marshal::StringToHGlobalAnsi(strLot_ID);
	char* chrINI = (char*)(void*)Marshal::StringToHGlobalAnsi(strINI);
	
	String^ nowTestInstance = this->Test_Retest->Text;
	String^ strNextTest = setRetestInstance(nowTestInstance);
	char* chrNextTest = (char*)(void*)Marshal::StringToHGlobalAnsi(strNextTest);

	// Use unmanaged string to update .INI information from unmanaged C code
	int OKd = delete_private_profile_section(chrLotID, chrINI);
	int OKw = write_private_profile_string(chrLotID, "Retest", chrNextTest, chrINI);
	Marshal::FreeHGlobal((IntPtr)(chrLotID)); // Free the unmanaged string
	Marshal::FreeHGlobal((IntPtr)(chrINI));
	Marshal::FreeHGlobal((IntPtr)(chrNextTest));

}

private: String^ setRetestInstance(String^ nowTestInstance)
{
	int intRetest;
	
	if (nowTestInstance == "None" || this->checkBox1->Checked == true)
		intRetest = 0;
		else if (nowTestInstance == "Full Test")
			intRetest = 1;
			else
			{
				int intptr = nowTestInstance->IndexOf(" ") + 1;
				nowTestInstance = nowTestInstance->Remove(0, intptr);
				intRetest = System::Convert::ToInt32(nowTestInstance) + 1;
			}
	return intRetest.ToString();
}

private: void AddToBin(Hashtable ^hashBins, String^ Bin_Number, String^ Fail_Count)
{
	if (hashBins->Contains(Bin_Number))
	{
		int intTotFailCount = System::Convert::ToInt32(hashBins[Bin_Number]);
		intTotFailCount += System::Convert::ToInt32(Fail_Count);
		hashBins[Bin_Number] = intTotFailCount.ToString();
	}
	else
		hashBins->Add(Bin_Number, Fail_Count);
}

private: bool loadTestFile(String^ testType, String^ strDataFile, 
			 DataGridView^ dataGridView)
{
	
	StreamReader^ din;
	String^ str; 
	array<String^>^ s = gcnew array<String^>(8);
	array<Char>^chars =  {','};
	array<Char>^ chrComment = {';','0'}; 	
	array<String^>^ strTestType = {"Final Test", "Sort Entries"};
	
	try 
	{
		din = File::OpenText(strDataFile);
	}
	catch (Exception^ e)
	{
		if (dynamic_cast<FileNotFoundException^>(e))
			MessageBox::Show(testType + " data file was not found. \n\n" + 
			"Note: PROMIS and Test files must reside in the same directory. \n\n" +  
			"Program will close.", 
			"File Not Found", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);				
		else
			MessageBox::Show(testType + " data file cannot be opened. Program will close.", 
			"File Corrupted", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		return false;
		}
	//
	// Record contents: Enabled, Family, Tests name, Test#, Mapped Test, Bin#, Condition, Comment
	//
	int counter = 0;
	while ((str = din->ReadLine()) != nullptr) 
	{	
		counter ++;
		if (str->IndexOfAny(chrComment) != 0 && String::IsNullOrEmpty(str) == false) 
		{
			array<String^>^rawFT = str->Split(chars);
			int colCnt = dataGridView3->ColumnCount;	
			for (int i = 1; i < colCnt; i++)
			{
				if ( i > 1 )     // Tests name, Test#, Mapped Test and Bin# must be present
				{
					if (String::IsNullOrEmpty(rawFT[i]) == true)
					{
						MessageBox::Show(testType + " CSV data file contains an incomplete record at line = "
							+ counter.ToString() + "\n\n An entry for field: " + neededFields[i-2] + " is missing.\n\n" 
							+ "Program will close.", "File Corrupted", MessageBoxButtons::OK, 
							MessageBoxIcon::Exclamation);
						return false;
					}
				}
				s[i-1] = rawFT[i];	// Ignore "Enabled" field
			}
			s[colCnt-1] = "0";
			dataGridView->Rows->Add(s);
		}
	}	
	dataGridView->AutoResizeColumns();
	return true;
}

private: bool loadEquipmentIDs(String^ strDataFile, ComboBox^ comboBox)
{
	StreamReader^ din;
	String^ str; 
	String^ EquipID;
	array<Char>^chars =  {','};
	array<Char>^ chrComment = {';','0'}; 	
		
	try 
	{
		din = File::OpenText(strDataFile);
	}
	catch (Exception^ e)
	{
		if (dynamic_cast<FileNotFoundException^>(e))
			MessageBox::Show(strDataFile + " equipment ID data file was not found. \n\n" + 
			"Note: Equipment, PROMIS and Test files must reside in the same directory. \n\n" +
			"Program will close.\n\n", "File Not Found", 
			MessageBoxButtons::OK, MessageBoxIcon::Information);				
		else
			MessageBox::Show(strDataFile + " equipment ID data file cannot be opened.\n\n" + 
			"Program will close.\n\n", "File Corrupted", 
			MessageBoxButtons::OK, MessageBoxIcon::Exclamation); 
		return false;
	}
	//
	// Record contents: Equipment ID, Descripiton
	//
	comboBox->BeginUpdate();
	comboBox->Items->Clear();
	comboBox->Items->Add( "" );
	while ((str = din->ReadLine()) != nullptr) 
	{	
		if (str->IndexOfAny(chrComment) != 0 && String::IsNullOrEmpty(str) == false) 
		{
			array<String^>^rawFT = str->Split(chars);			
			EquipID = rawFT[0];		// Need only the Equipment ID (the first field)
			comboBox->Items->Add( EquipID );
		}
	}	
	comboBox->EndUpdate();	
	return true;
}

private: bool isMultiDie(String^ strPkg_Type)
//	1S = single die, 2D = 2 dies, 2P = 2 dies, 3T = 3 dies, 4P = 4 dies
{
	if (strPkg_Type == "2D" || strPkg_Type == "2P" || strPkg_Type == "3T" || strPkg_Type == "4P")
		return true;
	else
		return false;
}


private: System::Void dataGridView1_RowHeaderMouseDoubleClick(System::Object^  sender, 
			 System::Windows::Forms::DataGridViewCellMouseEventArgs^  e) 
{ 
	int row = dataGridView1->CurrentRow->Index;
	comboBox1->SelectedIndex = row;
	comboBox1_SelectionChangeCommitted(sender, e);
}

private: System::Void dataGridView1_CellMouseDoubleClick(System::Object^  sender, 
			 System::Windows::Forms::DataGridViewCellMouseEventArgs^  e) 
{
	int row = dataGridView1->CurrentRow->Index;
	comboBox1->SelectedIndex = row;
	comboBox1_SelectionChangeCommitted(sender, e);
}

};
}