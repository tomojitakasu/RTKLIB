object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'NTRIP Browser'
  ClientHeight = 322
  ClientWidth = 634
  Color = clBtnFace
  Constraints.MinHeight = 150
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu
  OldCreateOrder = False
  Scaled = False
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 634
    Height = 22
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    object BtnList: TSpeedButton
      Left = 3
      Top = 1
      Width = 21
      Height = 21
      Flat = True
      Glyph.Data = {
        DE000000424DDE0000000000000076000000280000000D0000000D0000000100
        0400000000006800000000000000000000001000000000000000000000000000
        8000008000000080800080000000800080008080000080808000C0C0C0000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
        F000FFFFFFFFFFFFF000FFFF00007F0FF000FFF0FFFF000FF000FF7FFFF7000F
        F000FFFFFFFFF00FF000FF0FFFFFFF0FF000FF00FFFFFFFFF000FF0007FFFF7F
        F000FF000FFFF0FFF000FF0F70000FFFF000FFFFFFFFFFFFF000FFFFFFFFFFFF
        F000}
      OnClick = BtnListClick
    end
    object BtnUpdate: TSpeedButton
      Left = 209
      Top = 1
      Width = 21
      Height = 21
      Flat = True
      Glyph.Data = {
        DE000000424DDE0000000000000076000000280000000D0000000D0000000100
        0400000000006800000000000000000000001000000000000000000000000000
        8000008000000080800080000000800080008080000080808000C0C0C0000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
        F000FFFFF8FFFFFFF000FFFF70FFFFFFF000FFF700FFFFFFF000FF7000778FFF
        F000F700000000FFF000FF700087008FF000FFF700FFF00FF000FFFF70FFF00F
        F000FFFFF8FFF00FF000FFFFFFFFF00FF000FFFFFFFFF00FF000FFFFFFFFFFFF
        F000}
      OnClick = BtnUpdateClick
    end
    object TypeStr: TSpeedButton
      Left = 250
      Top = 0
      Width = 25
      Height = 22
      GroupIndex = 1
      Down = True
      Caption = 'STR'
      Flat = True
      Spacing = 2
      OnClick = TypeStrClick
    end
    object TypeCas: TSpeedButton
      Left = 276
      Top = 0
      Width = 27
      Height = 22
      GroupIndex = 1
      Caption = 'CAS'
      Flat = True
      Spacing = 2
      OnClick = TypeCasClick
    end
    object TypeNet: TSpeedButton
      Left = 304
      Top = 0
      Width = 25
      Height = 22
      GroupIndex = 1
      Caption = 'NET'
      Flat = True
      Spacing = 2
      OnClick = TypeNetClick
    end
    object TypeSrc: TSpeedButton
      Left = 330
      Top = 0
      Width = 25
      Height = 22
      GroupIndex = 1
      Caption = 'SRC'
      Flat = True
      Spacing = 2
      OnClick = TypeSrcClick
    end
    object BtnMap: TSpeedButton
      Left = 356
      Top = 0
      Width = 25
      Height = 22
      Caption = 'MAP'
      Flat = True
      OnClick = BtnMapClick
    end
    object Address: TComboBox
      Left = 25
      Top = 1
      Width = 184
      Height = 21
      DropDownCount = 40
      TabOrder = 0
      OnChange = AddressChange
      OnKeyPress = AddressKeyPress
    end
    object StaMask: TCheckBox
      Left = 463
      Top = 3
      Width = 71
      Height = 17
      Caption = 'STA'
      TabOrder = 1
      Visible = False
      OnClick = StaMaskClick
    end
    object BtnSta: TButton
      Left = 501
      Top = 1
      Width = 22
      Height = 21
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -9
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 2
      Visible = False
      OnClick = BtnStaClick
    end
    object FiltFmt: TComboBox
      Left = 390
      Top = 0
      Width = 66
      Height = 21
      TabOrder = 3
      Visible = False
      Items.Strings = (
        ''
        'RTCM 3'
        'RTCM 2'
        'RAW')
    end
  end
  object Table2: TStringGrid
    Left = 0
    Top = 22
    Width = 634
    Height = 282
    Margins.Left = 1
    Margins.Top = 1
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    ColCount = 18
    DefaultRowHeight = 15
    FixedCols = 0
    RowCount = 100
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goColSizing, goEditing, goAlwaysShowEditor]
    TabOrder = 3
    OnMouseDown = Table2MouseDown
    ColWidths = (
      80
      111
      29
      28
      325
      337
      293
      405
      670
      9
      7
      9
      9
      9
      8
      9
      12
      10)
  end
  object Table3: TMemo
    Left = 0
    Top = 22
    Width = 634
    Height = 282
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    Lines.Strings = (
      '')
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 4
    WordWrap = False
  end
  object Table1: TStringGrid
    Left = 0
    Top = 22
    Width = 634
    Height = 282
    Margins.Left = 1
    Margins.Top = 1
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    ColCount = 18
    DefaultRowHeight = 15
    FixedCols = 0
    RowCount = 100
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goColSizing, goEditing, goAlwaysShowEditor]
    TabOrder = 2
    OnMouseDown = Table1MouseDown
    ColWidths = (
      112
      40
      97
      127
      22
      29
      50
      51
      142
      45
      699
      7
      7
      6
      7
      8
      6
      6)
  end
  object Table0: TStringGrid
    Left = 0
    Top = 22
    Width = 634
    Height = 282
    Margins.Left = 1
    Margins.Top = 1
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    ColCount = 18
    DefaultRowHeight = 15
    FixedCols = 0
    RowCount = 100
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goColSizing, goEditing]
    TabOrder = 1
    OnMouseDown = Table0MouseDown
    OnSelectCell = Table0SelectCell
    ColWidths = (
      74
      117
      56
      240
      22
      52
      62
      28
      51
      50
      18
      19
      125
      28
      27
      27
      43
      623)
  end
  object Panel2: TPanel
    Left = 0
    Top = 304
    Width = 634
    Height = 18
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 5
    object Panel3: TPanel
      Left = 0
      Top = 0
      Width = 634
      Height = 18
      Align = alClient
      BevelInner = bvLowered
      BevelOuter = bvNone
      BorderWidth = 1
      TabOrder = 0
      object Message: TLabel
        Left = 4
        Top = 2
        Width = 3
        Height = 13
      end
    end
  end
  object SaveDialog: TSaveDialog
    Filter = 'All File (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 558
    Top = 274
  end
  object OpenDialog: TOpenDialog
    Filter = 'All File (*.*)|*.*'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 526
    Top = 274
  end
  object MainMenu: TMainMenu
    Left = 498
    Top = 274
    object File1: TMenuItem
      Caption = '&File'
      object MenuOpen: TMenuItem
        Caption = '&Open Source Table...'
        ShortCut = 16463
        OnClick = MenuOpenClick
      end
      object MenuSave: TMenuItem
        Caption = '&Save Source Table...'
        ShortCut = 16467
        OnClick = MenuSaveClick
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object MenuQuit: TMenuItem
        Caption = '&Quit'
        ShortCut = 16465
        OnClick = MenuQuitClick
      end
    end
    object Edit1: TMenuItem
      Caption = '&Edit'
      object MenuUpdateCaster: TMenuItem
        Caption = 'Update &Caster List'
        OnClick = MenuUpdateCasterClick
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object MenuUpdateTable: TMenuItem
        Caption = 'Update Source &Table'
        ShortCut = 16469
        OnClick = MenuUpdateTableClick
      end
    end
    object View1: TMenuItem
      Caption = '&View'
      ShortCut = 16467
      object MenuViewStr: TMenuItem
        AutoCheck = True
        Caption = '&Stream List'
        Checked = True
        OnClick = MenuViewStrClick
      end
      object MenuViewCas: TMenuItem
        AutoCheck = True
        Caption = '&Caster List'
        OnClick = MenuViewCasClick
      end
      object MenuViewNet: TMenuItem
        AutoCheck = True
        Caption = '&Network List'
        OnClick = MenuViewNetClick
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object MenuViewSrc: TMenuItem
        AutoCheck = True
        Caption = '&Source Table'
        OnClick = MenuViewSrcClick
      end
    end
    object Help1: TMenuItem
      Caption = '&Help'
      object MenuAbout: TMenuItem
        Caption = '&About...'
        OnClick = MenuAboutClick
      end
    end
  end
  object Timer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = TimerTimer
    Left = 468
    Top = 276
  end
end
