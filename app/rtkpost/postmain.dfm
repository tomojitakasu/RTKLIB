object MainForm: TMainForm
  Left = 0
  Top = 0
  AutoSize = True
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'RTKPOST'
  ClientHeight = 335
  ClientWidth = 466
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 466
    Height = 307
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    object Message: TPanel
      Left = 2
      Top = 275
      Width = 463
      Height = 23
      BevelInner = bvRaised
      BevelOuter = bvLowered
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      object BtnAbout: TSpeedButton
        Left = 438
        Top = 2
        Width = 21
        Height = 18
        Caption = '?'
        Flat = True
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clGray
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        OnClick = BtnAboutClick
      end
      object BtnOutputView2: TSpeedButton
        Left = 23
        Top = 2
        Width = 21
        Height = 18
        Hint = 'Show Debug Trace'
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        ParentShowHint = False
        ShowHint = True
        OnClick = BtnOutputView2Click
      end
      object BtnOutputView1: TSpeedButton
        Left = 2
        Top = 2
        Width = 21
        Height = 18
        Hint = 'Show Stat File'
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        ParentShowHint = False
        ShowHint = True
        OnClick = BtnOutputView1Click
      end
    end
    object Progress: TProgressBar
      Left = 2
      Top = 300
      Width = 462
      Height = 7
      Smooth = True
      Step = 1
      TabOrder = 4
    end
    object Panel3: TPanel
      Left = 2
      Top = 2
      Width = 463
      Height = 43
      BevelInner = bvRaised
      BevelOuter = bvLowered
      TabOrder = 0
      object LabelTimeInt: TLabel
        Left = 382
        Top = 21
        Width = 5
        Height = 13
        Caption = 's'
      end
      object LabelTimeUnit: TLabel
        Left = 448
        Top = 21
        Width = 7
        Height = 13
        Caption = 'H'
      end
      object BtnTime1: TSpeedButton
        Left = 122
        Top = 1
        Width = 17
        Height = 17
        Caption = '?'
        Flat = True
        OnClick = BtnTime1Click
      end
      object BtnTime2: TSpeedButton
        Left = 280
        Top = 1
        Width = 17
        Height = 17
        Caption = '?'
        Flat = True
        OnClick = BtnTime2Click
      end
      object TimeStart: TCheckBox
        Left = 4
        Top = 2
        Width = 105
        Height = 17
        Caption = 'Time Start (GPST)'
        TabOrder = 0
        OnClick = TimeStartClick
      end
      object TimeY1: TEdit
        Left = 4
        Top = 18
        Width = 65
        Height = 21
        TabOrder = 1
        Text = '2000/01/01'
      end
      object TimeH1: TEdit
        Left = 86
        Top = 18
        Width = 55
        Height = 21
        TabOrder = 3
        Text = '00:00:00'
      end
      object TimeEnd: TCheckBox
        Left = 164
        Top = 2
        Width = 101
        Height = 17
        Caption = 'Time End (GPST)'
        TabOrder = 5
        OnClick = TimeStartClick
      end
      object TimeH1UD: TUpDown
        Left = 141
        Top = 17
        Width = 17
        Height = 23
        Min = -32000
        Max = 32000
        TabOrder = 4
        Wrap = True
        OnChangingEx = TimeH1UDChangingEx
      end
      object TimeY2: TEdit
        Left = 164
        Top = 18
        Width = 65
        Height = 21
        TabOrder = 6
        Text = '2099/12/31'
      end
      object TimeY2UD: TUpDown
        Left = 229
        Top = 17
        Width = 17
        Height = 23
        Min = -32000
        Max = 32000
        TabOrder = 7
        OnChangingEx = TimeY2UDChangingEx
      end
      object TimeH2: TEdit
        Left = 246
        Top = 18
        Width = 55
        Height = 21
        TabOrder = 8
        Text = '23:59:59'
      end
      object TimeH2UD: TUpDown
        Left = 301
        Top = 17
        Width = 17
        Height = 23
        Min = -32000
        Max = 32000
        TabOrder = 9
        Wrap = True
        OnChangingEx = TimeH2UDChangingEx
      end
      object TimeIntF: TCheckBox
        Left = 331
        Top = 2
        Width = 61
        Height = 17
        Caption = 'Interval'
        TabOrder = 10
        OnClick = TimeIntFClick
      end
      object TimeUnitF: TCheckBox
        Left = 396
        Top = 2
        Width = 45
        Height = 17
        Caption = 'Unit'
        TabOrder = 12
        OnClick = TimeUnitFClick
      end
      object TimeUnit: TEdit
        Left = 396
        Top = 18
        Width = 47
        Height = 21
        TabOrder = 13
        Text = '24'
      end
      object TimeY1UD: TUpDown
        Left = 69
        Top = 17
        Width = 17
        Height = 23
        Min = -32000
        Max = 32000
        TabOrder = 2
        Wrap = True
        OnChangingEx = TimeY1UDChangingEx
      end
      object TimeInt: TComboBox
        Left = 331
        Top = 18
        Width = 47
        Height = 21
        DropDownCount = 20
        TabOrder = 11
        Text = '0'
        Items.Strings = (
          '0'
          '0.05'
          '0.1'
          '0.2'
          '0.25'
          '0.5'
          '1'
          '2'
          '5'
          '10'
          '15'
          '30'
          '60')
      end
    end
    object Panel4: TPanel
      Left = 2
      Top = 45
      Width = 463
      Height = 182
      BevelInner = bvRaised
      BevelOuter = bvLowered
      TabOrder = 1
      object LabelInputFile1: TLabel
        Left = 6
        Top = 4
        Width = 53
        Height = 13
        Caption = 'RINEX OBS'
      end
      object BtnInputPlot1: TSpeedButton
        Left = 413
        Top = 2
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF000000000000FFFFFF808080FF
          FFFF000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF000000FFFFFFFFFF
          FFFFFFFF808080FFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFF00FFFFFFFFFFFF
          000000FFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFFFF000000FFFFFFFFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000080808080808080808080808080808080
          8080808080808080808080000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FF000000808080FFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFFFFFFFF
          000000FFFFFF000000FFFFFF000000FFFFFFFFFFFFFFFFFF000000FFFFFFFFFF
          FF00FFFFFFFFFFFF000000FFFFFFFFFFFF000000808080FFFFFFFFFFFFFFFFFF
          000000FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF000000000000FFFFFF808080FF
          FFFF000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputPlot1Click
      end
      object BtnInputView1: TSpeedButton
        Left = 438
        Top = 2
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputView1Click
      end
      object BtnInputView3: TSpeedButton
        Left = 366
        Top = 75
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputView3Click
      end
      object LabelInputFile2: TLabel
        Left = 6
        Top = 78
        Width = 260
        Height = 13
        Caption = 'RINEX NAV/CLK, SP3, FCB, IONEX, SBS/EMS  or RTCM'
      end
      object BtnInputView2: TSpeedButton
        Left = 438
        Top = 38
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputView2Click
      end
      object BtnInputPlot2: TSpeedButton
        Left = 413
        Top = 38
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF000000000000FFFFFF808080FF
          FFFF000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF000000FFFFFFFFFF
          FFFFFFFF808080FFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFF00FFFFFFFFFFFF
          000000FFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFFFF000000FFFFFFFFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF808080FFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000080808080808080808080808080808080
          8080808080808080808080000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FF000000808080FFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFFFFFFFF
          000000FFFFFF000000FFFFFF000000FFFFFFFFFFFFFFFFFF000000FFFFFFFFFF
          FF00FFFFFFFFFFFF000000FFFFFFFFFFFF000000808080FFFFFFFFFFFFFFFFFF
          000000FFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF000000000000FFFFFF808080FF
          FFFF000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputPlot2Click
      end
      object LabelInputFile3: TLabel
        Left = 6
        Top = 40
        Width = 120
        Height = 13
        Caption = 'RINEX OBS: Base Station'
      end
      object BtnInputView4: TSpeedButton
        Left = 388
        Top = 75
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputView4Click
      end
      object BtnInputView5: TSpeedButton
        Left = 413
        Top = 75
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputView5Click
      end
      object BtnKeyword: TSpeedButton
        Left = 163
        Top = 1
        Width = 17
        Height = 16
        Caption = '?'
        Flat = True
        OnClick = BtnKeywordClick
      end
      object BtnInputView6: TSpeedButton
        Left = 438
        Top = 75
        Width = 21
        Height = 17
        Flat = True
        Glyph.Data = {
          3E020000424D3E0200000000000036000000280000000D0000000D0000000100
          1800000000000802000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080FFFFFFFFFFFFFFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF000000FFFFFF808080808080808080FFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF00FFFFFF000000
          FFFFFF808080808080808080808080808080808080808080FFFFFF000000FFFF
          FF00FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFF00FFFFFF00000000000000000000000000000000000000
          0000000000000000000000000000FFFFFF00FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF00}
        OnClick = BtnInputView6Click
      end
      object InputFile1: TComboBox
        Left = 4
        Top = 18
        Width = 433
        Height = 21
        DropDownCount = 20
        TabOrder = 0
        OnChange = InputFile1Change
      end
      object BtnInputFile1: TButton
        Left = 438
        Top = 18
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnInputFile1Click
      end
      object BtnInputFile3: TButton
        Left = 438
        Top = 92
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnInputFile3Click
      end
      object InputFile3: TComboBox
        Left = 4
        Top = 92
        Width = 433
        Height = 21
        DropDownCount = 20
        TabOrder = 4
      end
      object BtnInputFile2: TButton
        Left = 438
        Top = 54
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnClick = BtnInputFile2Click
      end
      object InputFile2: TComboBox
        Left = 4
        Top = 54
        Width = 433
        Height = 21
        DropDownCount = 20
        TabOrder = 2
      end
      object InputFile4: TComboBox
        Left = 4
        Top = 114
        Width = 433
        Height = 21
        DropDownCount = 20
        TabOrder = 6
      end
      object BtnInputFile4: TButton
        Left = 438
        Top = 114
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 7
        OnClick = BtnInputFile4Click
      end
      object InputFile5: TComboBox
        Left = 4
        Top = 136
        Width = 433
        Height = 21
        DropDownCount = 20
        TabOrder = 8
      end
      object BtnInputFile5: TButton
        Left = 438
        Top = 136
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 9
        OnClick = BtnInputFile5Click
      end
      object InputFile6: TComboBox
        Left = 4
        Top = 158
        Width = 433
        Height = 21
        DropDownCount = 20
        TabOrder = 10
      end
      object BtnInputFile6: TButton
        Left = 438
        Top = 158
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 11
        OnClick = BtnInputFile6Click
      end
    end
    object Panel5: TPanel
      Left = 2
      Top = 227
      Width = 463
      Height = 48
      BevelInner = bvRaised
      BevelOuter = bvLowered
      TabOrder = 2
      object Label1: TLabel
        Left = 6
        Top = 6
        Width = 38
        Height = 13
        Caption = 'Solution'
      end
      object LabelOutDir: TLabel
        Left = 65
        Top = 6
        Width = 13
        Height = 13
        Caption = 'Dir'
      end
      object OutDirEna: TCheckBox
        Left = 48
        Top = 4
        Width = 15
        Height = 17
        TabOrder = 0
        OnClick = OutDirEnaClick
      end
      object OutputFile: TComboBox
        Left = 4
        Top = 24
        Width = 433
        Height = 21
        DropDownCount = 20
        TabOrder = 3
      end
      object BtnOutputFile: TButton
        Left = 438
        Top = 24
        Width = 21
        Height = 21
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 4
        OnClick = BtnOutputFileClick
      end
      object OutDir: TEdit
        Left = 82
        Top = 2
        Width = 355
        Height = 21
        Enabled = False
        TabOrder = 1
        OnChange = OutDirChange
      end
      object BtnOutDir: TButton
        Left = 438
        Top = 2
        Width = 21
        Height = 21
        Caption = '...'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -9
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnOutDirClick
      end
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 307
    Width = 466
    Height = 28
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 1
    ExplicitTop = 305
    ExplicitWidth = 444
    ExplicitHeight = 25
    object BtnPlot: TButton
      Left = 1
      Top = 2
      Width = 78
      Height = 27
      Caption = '&Plot...'
      TabOrder = 0
      OnClick = BtnPlotClick
    end
    object BtnView: TButton
      Left = 78
      Top = 2
      Width = 78
      Height = 27
      Caption = '&View...'
      TabOrder = 1
      OnClick = BtnViewClick
    end
    object BtnToKML: TButton
      Left = 156
      Top = 2
      Width = 78
      Height = 27
      Caption = 'To &KML...'
      TabOrder = 2
      OnClick = BtnToKMLClick
    end
    object BtnOption: TButton
      Left = 233
      Top = 2
      Width = 78
      Height = 27
      Caption = '&Options...'
      TabOrder = 3
      OnClick = BtnOptionClick
    end
    object BtnExec: TButton
      Left = 310
      Top = 2
      Width = 78
      Height = 27
      Caption = 'E&xecute'
      TabOrder = 4
      OnClick = BtnExecClick
    end
    object BtnExit: TButton
      Left = 387
      Top = 2
      Width = 78
      Height = 27
      Caption = '&Exit'
      TabOrder = 5
      OnClick = BtnExitClick
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'All (*.*)|*.*|RINEX OBS (*.obs,*.*O,*.*D)|*.obs;*.*O;*.*D|RINEX ' +
      'NAV (*.*nav,*.*N,*.*P,*.*G,*.*H,*.*Q)|*.*nav;*.*N;*.*P;*.*G;*.*H' +
      ';*.*Q|Precise Ephemeris/Clock (*.sp3,*.eph*,*.clk*)|*.sp3;*eph*;' +
      '*.clk*|FCB (*.fcb),IONEX (*.*i,*.ionex),SBAS (*.sbs,*.ems)|*.fcb' +
      ';*.*i;*.ionex;*.sbs;*.ems'
    Options = [ofHideReadOnly, ofNoChangeDir, ofAllowMultiSelect, ofEnableSizing]
    Left = 334
    Top = 235
  end
  object SaveDialog: TSaveDialog
    Filter = 'All Files (*.*)|*.*|Position Files (*.pos)|*.pos'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 306
    Top = 235
  end
end
